/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Rokas Kupstys
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

class ImGUI
    : public Atomic::Object
{
    ATOMIC_OBJECT(ImGUI, Atomic::Object);
public:
    ImGUI(Atomic::Context* context);
    ~ImGUI();

    //! Get ui scale.
    //! \return scale of ui.
    float GetScale() const { return _uiScale; };
    //! Set ui scale.
    //! \param scale of ui.
    void SetScale(float scale);
    //! Add font to imgui subsystem.
    /*!
      \param font_path a string pointing to TTF font resource.
      \param size a font size. If 0 then size of last font is used.
      \param ranges optional ranges of font that should be used. Parameter is array of {start1, stop1, ..., startN, stopN, 0}.
      \param merge set to true if new font should be merged to last active font.
      \return ImFont instance that may be used for setting current font when drawing GUI.
    */
    ImFont* AddFont(const Atomic::String& font_path, float size=0, const unsigned short* ranges=0, bool merge=false);
    //! Add font to imgui subsystem.
    /*!
      \param font_path a string pointing to TTF font resource.
      \param size a font size. If 0 then size of last font is used.
      \param ranges optional ranges of font that should be used. Parameter is std::initializer_list of {start1, stop1, ..., startN, stopN, 0}.
      \param merge set to true if new font should be merged to last active font.
      \return ImFont instance that may be used for setting current font when drawing GUI.
    */
    ImFont* AddFont(const Atomic::String& font_path, float size=0,
                    const std::initializer_list<unsigned short>& ranges={}, bool merge=false);

protected:
    float _uiScale = 1.f;
    Atomic::Matrix4 _projection;
    Atomic::VertexBuffer _vertex_buffer;
    Atomic::IndexBuffer _index_buffer;
    Atomic::SharedPtr<Atomic::Texture2D> _font_texture;

    void ReallocateFontTexture();
    void UpdateProjectionMatrix();
    void OnPostUpdate(Atomic::VariantMap& args);
    void OnRenderDrawLists(ImDrawData* data);
    void OnRawEvent(VariantMap& args);
};

}
