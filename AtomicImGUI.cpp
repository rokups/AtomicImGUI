/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Rokas Kupstys
 * Copyright (c) 2016 Yehonatan Ballas
 * Copyright (c) 2008-2016 the Urho3D project.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#define STB_RECT_PACK_IMPLEMENTATION
#include "AtomicImGUI.h"

#include <Atomic/Input/InputEvents.h>
#include <Atomic/Input/Input.h>
#include <Atomic/Core/CoreEvents.h>
#include <Atomic/Engine/EngineEvents.h>
#include <Atomic/Graphics/GraphicsEvents.h>
#include <Atomic/Graphics/Graphics.h>
#include <Atomic/Core/Profiler.h>
#include <Atomic/Resource/ResourceCache.h>
#include <SDL.h>

using namespace Atomic;
using namespace std::placeholders;


AtomicImGUI::AtomicImGUI(Atomic::Context* context)
    : Object(context)
    , _vertex_buffer(context)
    , _index_buffer(context)
{
    ImGuiIO& io = ImGui::GetIO();
    io.KeyMap[ImGuiKey_Tab] = SCANCODE_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = SCANCODE_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = SCANCODE_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = SCANCODE_UP;
    io.KeyMap[ImGuiKey_DownArrow] = SCANCODE_DOWN;
    io.KeyMap[ImGuiKey_Home] = SCANCODE_HOME;
    io.KeyMap[ImGuiKey_End] = SCANCODE_END;
    io.KeyMap[ImGuiKey_Delete] = SCANCODE_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = SCANCODE_BACKSPACE;
    io.KeyMap[ImGuiKey_Enter] = SCANCODE_RETURN;
    io.KeyMap[ImGuiKey_Escape] = SCANCODE_ESCAPE;
    io.KeyMap[ImGuiKey_A] = SCANCODE_A;
    io.KeyMap[ImGuiKey_C] = SCANCODE_C;
    io.KeyMap[ImGuiKey_V] = SCANCODE_V;
    io.KeyMap[ImGuiKey_X] = SCANCODE_X;
    io.KeyMap[ImGuiKey_Y] = SCANCODE_Y;
    io.KeyMap[ImGuiKey_Z] = SCANCODE_Z;

    io.RenderDrawListsFn = [](ImDrawData* data) { static_cast<AtomicImGUI*>(ImGui::GetIO().UserData)->OnRenderDrawLists(data); };
    io.SetClipboardTextFn = [](const char* text) { SDL_SetClipboardText(text); };
    io.GetClipboardTextFn = []() -> const char* { return SDL_GetClipboardText(); };

    io.UserData = this;

    io.Fonts->AddFontDefault();
    ReallocateFontTexture();
    OnScreenModeChange();

    // Subscribe to events
    SubscribeToEvent(E_POSTUPDATE, std::bind(&AtomicImGUI::OnPostUpdate, this, _2));
    SubscribeToEvent(E_ENDRENDERING, [&](StringHash, VariantMap&) {
        ATOMIC_PROFILE(ImGuiRender);
        ImGui::Render();
    });
    SubscribeToEvent(E_KEYUP, std::bind(&AtomicImGUI::OnKeyUp, this, _2));
    SubscribeToEvent(E_KEYDOWN, std::bind(&AtomicImGUI::OnKeyDown, this, _2));
    SubscribeToEvent(E_TEXTINPUT, std::bind(&AtomicImGUI::OnTextInput, this, _2));
    SubscribeToEvent(E_TOUCHBEGIN, std::bind(&AtomicImGUI::OnTouchBegin, this, _2));
    SubscribeToEvent(E_TOUCHEND, std::bind(&AtomicImGUI::OnTouchEnd, this, _2));
    SubscribeToEvent(E_TOUCHMOVE, std::bind(&AtomicImGUI::OnTouchMove, this, _2));
    SubscribeToEvent(E_SCREENMODE, std::bind(&AtomicImGUI::OnScreenModeChange, this));
}

void AtomicImGUI::OnScreenModeChange()
{
    // Update screen size
    auto graphics = GetSubsystem<Graphics>();
    ImGui::GetIO().DisplaySize = ImVec2((float)graphics->GetWidth(), (float)graphics->GetHeight());

    // Update projection matrix
    IntVector2 viewSize = graphics->GetViewport().Size();
    Vector2 invScreenSize(1.0f / viewSize.x_, 1.0f / viewSize.y_);
    Vector2 scale(2.0f * invScreenSize.x_, -2.0f * invScreenSize.y_);
    Vector2 offset(-1.0f, 1.0f);

    _projection = Matrix4(Matrix4::IDENTITY);
    _projection.m00_ = scale.x_ /** _uiScale*/;
    _projection.m03_ = offset.x_;
    _projection.m11_ = scale.y_ /** _uiScale*/;
    _projection.m13_ = offset.y_;
    _projection.m22_ = 1.0f;
    _projection.m23_ = 0.0f;
    _projection.m33_ = 1.0f;
}

void AtomicImGUI::OnPostUpdate(VariantMap& args)
{
    auto& io = ImGui::GetIO();
    float timeStep = args[PostUpdate::P_TIMESTEP].GetFloat();
    io.DeltaTime = timeStep > 0.0f ? timeStep : 1.0f / 60.0f;

    // mouse input
    auto input = GetSubsystem<Input>();
    if (input->IsMouseVisible() && !input->GetTouchEmulation())
    {
        IntVector2 pos = input->GetMousePosition();
        // Mouse position, in pixels (set to -1,-1 if no mouse / on another screen, etc.)
        io.MousePos.x = (float)pos.x_;
        io.MousePos.y = (float)pos.y_;
    }
    else
    {
        io.MousePos.x = -1.0f;
        io.MousePos.y = -1.0f;
    }

    io.MouseDown[0] = input->GetMouseButtonDown(MOUSEB_LEFT);
    io.MouseDown[1] = input->GetMouseButtonDown(MOUSEB_RIGHT);
    io.MouseDown[2] = input->GetMouseButtonDown(MOUSEB_MIDDLE);
    io.MouseWheel = (float)input->GetMouseMoveWheel();

    // Modifiers
    io.KeyCtrl = input->GetQualifierDown(QUAL_CTRL);
    io.KeyShift = input->GetQualifierDown(QUAL_SHIFT);
    io.KeyAlt = input->GetQualifierDown(QUAL_ALT);

    // Start new ImGui frame
    ImGui::NewFrame();

    // Send imgui NewFrame events, so users can use it as a guarenteed way to use imgui after NewFrame() was called.
    ATOMIC_PROFILE(ImGuiFrame);
    SendEvent(E_IMGUIFRAME);
}

void AtomicImGUI::OnRenderDrawLists(ImDrawData* data)
{
    auto _graphics = GetSubsystem<Graphics>();
    // Engine does not render when window is closed or device is lost
    assert(_graphics && _graphics->IsInitialized() && !_graphics->IsDeviceLost());


    ImGuiIO& io = ImGui::GetIO();
    int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
    int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
    if (fb_width == 0 || fb_height == 0)
        return;
    data->ScaleClipRects(io.DisplayFramebufferScale);

    for (int n = 0; n < data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = data->CmdLists[n];
        unsigned int idx_buffer_offset = 0;

        // Resize vertex and index buffers on the fly. Once buffer becomes too small for data that is to be rendered
        // we reallocate buffer to be twice as big as we need now. This is done in order to minimize memory reallocation
        // in rendering loop.
        if (cmd_list->VtxBuffer.Size > _vertex_buffer.GetVertexCount())
        {
            PODVector<VertexElement> elems = { VertexElement(TYPE_VECTOR2, SEM_POSITION),
                                               VertexElement(TYPE_VECTOR2, SEM_TEXCOORD),
                                               VertexElement(TYPE_UBYTE4_NORM, SEM_COLOR)
            };
            _vertex_buffer.SetSize((unsigned int)(cmd_list->VtxBuffer.Size * 2), elems, true);
        }
        if (cmd_list->IdxBuffer.Size > _index_buffer.GetIndexCount())
            _index_buffer.SetSize((unsigned int)(cmd_list->IdxBuffer.Size * 2), false, true);

        _vertex_buffer.SetDataRange(cmd_list->VtxBuffer.Data, 0, (unsigned int)cmd_list->VtxBuffer.Size, true);
        _index_buffer.SetDataRange(cmd_list->IdxBuffer.Data, 0, (unsigned int)cmd_list->IdxBuffer.Size, true);

        _graphics->ClearParameterSources();
        _graphics->SetColorWrite(true);
        _graphics->SetCullMode(CULL_NONE);
        _graphics->SetDepthTest(CMP_ALWAYS);
        _graphics->SetDepthWrite(false);
        _graphics->SetFillMode(FILL_SOLID);
        _graphics->SetStencilTest(false);
        _graphics->SetVertexBuffer(&_vertex_buffer);
        _graphics->SetIndexBuffer(&_index_buffer);

        for (const ImDrawCmd* cmd = cmd_list->CmdBuffer.begin(); cmd != cmd_list->CmdBuffer.end(); cmd++)
        {
            if (cmd->UserCallback)
                cmd->UserCallback(cmd_list, cmd);
            else
            {
                ShaderVariation* ps;
                ShaderVariation* vs;

                Texture2D* texture = static_cast<Texture2D*>(cmd->TextureId);
                if (!texture)
                {
                    ps = _graphics->GetShader(PS, "Basic", "VERTEXCOLOR");
                    vs = _graphics->GetShader(VS, "Basic", "VERTEXCOLOR");
                }
                else
                {
                    // If texture contains only an alpha channel, use alpha shader (for fonts)
                    vs = _graphics->GetShader(VS, "Basic", "DIFFMAP VERTEXCOLOR");
                    if (texture->GetFormat() == Graphics::GetAlphaFormat())
                        ps = _graphics->GetShader(PS, "Basic", "ALPHAMAP VERTEXCOLOR");
                    else
                        ps = _graphics->GetShader(PS, "Basic", "DIFFMAP VERTEXCOLOR");
                }

                _graphics->SetShaders(vs, ps);
                if (_graphics->NeedParameterUpdate(SP_OBJECT, this))
                    _graphics->SetShaderParameter(VSP_MODEL, Matrix3x4::IDENTITY);
                if (_graphics->NeedParameterUpdate(SP_CAMERA, this))
                    _graphics->SetShaderParameter(VSP_VIEWPROJ, _projection);
                if (_graphics->NeedParameterUpdate(SP_MATERIAL, this))
                    _graphics->SetShaderParameter(PSP_MATDIFFCOLOR, Color(1.0f, 1.0f, 1.0f, 1.0f));

                float elapsedTime = GetSubsystem<Time>()->GetElapsedTime();
                _graphics->SetShaderParameter(VSP_ELAPSEDTIME, elapsedTime);
                _graphics->SetShaderParameter(PSP_ELAPSEDTIME, elapsedTime);

                IntRect scissor = IntRect(int(cmd->ClipRect.x), int(cmd->ClipRect.y),
                                          int(cmd->ClipRect.x + cmd->ClipRect.z),
                                          int(cmd->ClipRect.y + cmd->ClipRect.w));
//                scissor.left_ = int(scissor.left_ * _uiScale);
//                scissor.top_ = int(scissor.top_ * _uiScale);
//                scissor.right_ = int(scissor.right_ * _uiScale);
//                scissor.bottom_ = int(scissor.bottom_ * _uiScale);

                _graphics->SetBlendMode(BLEND_ALPHA);
                _graphics->SetScissorTest(true, scissor);
                _graphics->SetTexture(0, texture);
                _graphics->Draw(TRIANGLE_LIST, idx_buffer_offset, cmd->ElemCount, 0, 0, _vertex_buffer.GetVertexCount());
                idx_buffer_offset += cmd->ElemCount;
            }
        }
    }
    _graphics->SetScissorTest(false);
}

ImFont* AtomicImGUI::AddFont(const String& font_path, float size, bool merge, const unsigned short* ranges)
{
    auto io = ImGui::GetIO();

    if (size == 0)
    {
        if (io.Fonts->Fonts.size() == 0)
            return 0;
        size = io.Fonts->Fonts.back()->FontSize;
    }

    if (auto font_file = GetSubsystem<ResourceCache>()->GetFile(font_path))
    {
        PODVector<uint8_t> data;
        data.Resize(font_file->GetSize());
        auto bytes_len = font_file->Read(&data.Front(), data.Size());
        ImFontConfig cfg;
        cfg.MergeMode = merge;
        if (auto new_font = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(&data.Front(), bytes_len, size, &cfg, ranges))
        {
            ReallocateFontTexture();
            return new_font;
        }
    }
    return 0;
}

void AtomicImGUI::ReallocateFontTexture()
{
    auto io = ImGui::GetIO();
    // Create font texture.
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bits for OpenGL3 demo because it is more likely to be compatible with user's existing shader.

    if (!_font_texture || width != _font_texture->GetWidth() || height != _font_texture->GetHeight())
    {
        _font_texture = new Texture2D(context_);
        _font_texture->SetNumLevels(1);
        _font_texture->SetSize(width, height, Graphics::GetRGBAFormat());
        _font_texture->SetData(0, 0, 0, width, height, pixels);
        _font_texture->SetFilterMode(FILTER_BILINEAR);

        // Store our identifier
        io.Fonts->TexID = (void*)_font_texture.Get();
        io.Fonts->ClearTexData();
    }
}

void AtomicImGUI::OnKeyUp(VariantMap& args)
{
    auto code = args[KeyUp::P_SCANCODE].GetInt();
    if (code < 512)
        ImGui::GetIO().KeysDown[code] = false;
}

void AtomicImGUI::OnKeyDown(VariantMap& args)
{
    auto code = args[KeyUp::P_SCANCODE].GetInt();
    if (code < 512)
        ImGui::GetIO().KeysDown[code] = true;
}

void AtomicImGUI::OnTextInput(VariantMap& args)
{
    ImGui::GetIO().AddInputCharactersUTF8(args[TextInput::P_TEXT].GetString().CString());
}

void AtomicImGUI::OnTouchBegin(VariantMap& args)
{
    using namespace TouchBegin;
    if (!_touching)
    {
        _touching = true;
        _touch_id = args[P_TOUCHID].GetInt();
        auto& io_ = ImGui::GetIO();
        io_.MousePos.x = (float)args[P_X].GetInt();
        io_.MousePos.y = (float)args[P_Y].GetInt();
        io_.MouseDown[0] = true;
    }
}

void AtomicImGUI::OnTouchEnd(VariantMap& args)
{
    using namespace TouchEnd;
    if (args[P_TOUCHID].GetInt() == _touch_id)
    {
        auto& io_ = ImGui::GetIO();
        io_.MousePos.x = (float)args[P_X].GetInt();
        io_.MousePos.y = (float)args[P_Y].GetInt();
        io_.MouseDown[0] = false;
    }
    _touching = false;
}

void AtomicImGUI::OnTouchMove(VariantMap& args)
{
    using namespace TouchMove;
    if (args[P_TOUCHID].GetInt() == _touch_id)
    {
        auto& io_ = ImGui::GetIO();
        io_.MousePos.x = (float)args[P_X].GetInt();
        io_.MousePos.y = (float)args[P_Y].GetInt();
    }
}
