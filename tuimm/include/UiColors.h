#pragma once

namespace UiColors
{
void init();
int highlightedStyle();
int focusedStyle();
int normalStyle();
int boldStyle();
int dimmedStyle();
int focusStyle(bool focused);
int highlightStyle(bool highlighted);
int getStyle(bool isCursor, bool isSelected);
}
