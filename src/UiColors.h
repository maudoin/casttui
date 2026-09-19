#pragma once

namespace UiColors
{
void init();
int highlightedStyle();
int focusedStyle();
int normalStyle();
int focusStyle(bool focused);
int highlightStyle(bool highlighted);
int getStyle(bool isCursor, bool isSelected);
}
