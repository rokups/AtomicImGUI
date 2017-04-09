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
#pragma once


#include <Atomic/Core/Object.h>
#include <Atomic/Math/StringHash.h>
#include <Atomic/Container/HashMap.h>
#include <Atomic/Graphics/VertexBuffer.h>
#include <Atomic/Graphics/IndexBuffer.h>
#include <Atomic/Math/Matrix4.h>
#include <Atomic/Graphics/Texture2D.h>

#include "imgui/imgui.h"

namespace Atomic
{

ATOMIC_EVENT(E_IMGUIFRAME, ImGUIFrame) { }

class AtomicImGUI
    : public Atomic::Object
{
    ATOMIC_OBJECT(AtomicImGUI, Atomic::Object);
public:
    AtomicImGUI(Atomic::Context* context);
    //! Add font to imgui subsystem.
    /*!
      \param font_path a string pointing to TTF font resource.
      \param size a font size. If 0 then size of last font is used.
      \param merge set to true if new font should be merged to last active font.
      \param ranges optional ranges of font that should be used. Parameter is array of {start1, stop1, ..., startN, stopN, 0}.
      \return ImFont instance that may be used for setting current font when drawing GUI.
    */
    ImFont* AddFont(const Atomic::String& font_path, float size=0, bool merge=false, const unsigned short* ranges=0);

protected:
    Atomic::Matrix4 _projection;
    Atomic::VertexBuffer _vertex_buffer;
    Atomic::IndexBuffer _index_buffer;
    Atomic::SharedPtr<Atomic::Texture2D> _font_texture;
    bool _touching = false;
    int _touch_id = -1;

    void ReallocateFontTexture();
    void OnScreenModeChange();
    void OnPostUpdate(Atomic::VariantMap& args);
    void OnRenderDrawLists(ImDrawData* data);
    void OnKeyUp(Atomic::VariantMap& args);
    void OnKeyDown(Atomic::VariantMap& args);
    void OnTextInput(Atomic::VariantMap& args);
    void OnTouchBegin(Atomic::VariantMap& args);
    void OnTouchEnd(Atomic::VariantMap& args);
    void OnTouchMove(Atomic::VariantMap& args);
};

}
