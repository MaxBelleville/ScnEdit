/*
!@
MIT License

Copyright (c) 2024 Skylicht Technology CO., LTD

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files
(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify,
merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

This file is part of the "Skylicht Engine".
https://github.com/skylicht-lab/skylicht-engine
!#
*/

#pragma once

#include "CUIBase.h"

namespace Skylicht
{
	namespace UI
	{
		class CUITextBox : public CUIBase
		{
		protected:
			CGUIElement* m_background;
			CGUIText* m_text;

			bool m_removeable;
			bool m_addable;
			bool m_completed;
			//-1 is any, 0 is none, 1 is shift
			core::array<std::pair<irr::EKEY_CODE, int>> m_acceptedKeys;

			int m_maxLength;
			int m_minLength;

		public:
			std::function<void(CUIBase*)> OnTextChanged;
			std::function<void(CUIBase*)> OnTextSet;

		public:
			CUITextBox(CUIContainer* container, CGUIElement* element);

			virtual ~CUITextBox();

			inline CGUIElement* getBackground()
			{
				return m_background;
			}

			inline CGUIText* getTextGUI()
			{
				return m_text;
			}

			void setText(const char* text);

			void setText(const wchar_t* text);

			const char* getText();

			const wchar_t* getTextW();

			int getTextLength();

			inline void setRemoveable(bool b)
			{
				m_removeable = b;
			}

			inline void setAddable(bool b)
			{
				m_addable = b;
			}

			inline bool isRemoveable()
			{
				return m_removeable;
			}
			inline bool isAddable()
			{
				return m_addable;
			}
			inline bool isCompleted()
			{
				return m_completed;
			}

			inline void addAcceptedKeys(std::pair<irr::EKEY_CODE, int> key) {
				m_acceptedKeys.push_back(key);
			}
			inline void setAcceptedKeys(core::array<std::pair<irr::EKEY_CODE, int>> keys) {
				m_acceptedKeys = keys;
			}
			inline void resetAcceptedKeys() {
				m_acceptedKeys.set_used(0);
			}

			inline void setMaxLength(int l)
			{
				m_maxLength = l;
			}

			inline void setLength(int minl, int maxl)
			{
				m_minLength = minl;
				m_maxLength = maxl;
			}

			inline int getMaxLength()
			{
				return m_maxLength;
			}

			inline void setMinLength(int l)
			{
				m_minLength = l;
			}

			inline int getMinLength()
			{
				return m_minLength;
			}

			virtual void onPointerHover(float pointerX, float pointerY);

			virtual void onPointerOut(float pointerX, float pointerY);

			virtual void onPointerDown(float pointerX, float pointerY);

			virtual void onPointerUp(float pointerX, float pointerY);

			virtual void onPointerMove(float pointerX, float pointerY);

			virtual void onLostFocus();

			virtual void onKeyEvent(const SEvent& event);
		};
	}
}