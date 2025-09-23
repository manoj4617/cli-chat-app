#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <iostream>
#include <thread>
#include <chrono>

int main() {
  using namespace ftxui;
  auto doc = vbox({
    hbox({
      text("top-left"),
      filler(),
      text("top-right"),
    }),
    filler(),
    hbox({
      filler(),
      text("pole"),
      filler(),
    }),
    filler(),
    hbox({
      text("bottom-left"),
      filler(),
      text("bottom-right")
    }),
  });
  auto screen = Screen::Create(Dimension::Full(), Dimension::Full());
  Render(screen, doc);
  screen.Print();
  getchar();

  return 0;
}