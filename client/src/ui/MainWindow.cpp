#include "MainWindow.h"

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Progress.H>

#include "files/CacheHandler.h"
#include "lib/pfd/portable-file-dialogs.h"

MainWindow::MainWindow(){
  window = new Fl_Window(
    UIConfig::WindowWidth,
    UIConfig::WindowHeight,
    UIConfig::WindowTitle);

  urlLabel = new Fl_Box(0, 10, UIConfig::WindowWidth, 20);
  constexpr int WINDOW_QUARTER = UIConfig::WindowWidth / 4;

  auto worldSelectLabel = new Fl_Box(10, 50, WINDOW_QUARTER - 10, 20, "Select a world:");
  worldSelectChoice = new Fl_Choice(WINDOW_QUARTER + 10, 50, WINDOW_QUARTER * 3 - 20, 20);
  auto savesPathLabel = new Fl_Box(10, 80, WINDOW_QUARTER - 10, 20, "Path to saves: ");
  savesPathInput = new Fl_Input(WINDOW_QUARTER + 10, 80, (WINDOW_QUARTER - 10) *2, 20);
  savesPathBrowseButton = new Fl_Button(WINDOW_QUARTER * 3, 80, WINDOW_QUARTER - 10, 20, "Browse...");
  scoutDirectoryButton = new Fl_Button(10, 110, UIConfig::WindowWidth - 20, 30, "Scout directory");

  statusProgress = new Fl_Progress(10, UIConfig::WindowHeight - 200, UIConfig::WindowWidth - 20, 30, "Status");

  syncButton = new Fl_Button(10, UIConfig::WindowHeight - 160, UIConfig::WindowWidth - 20, 70, "Sync!");
  uploadButton = new Fl_Button(10, UIConfig::WindowHeight - 80, UIConfig::WindowWidth - 20, 70, "Upload!");

  savesPathBrowseButton->callback([](Fl_Widget*, void*v) {
    auto* self = static_cast<MainWindow*>(v);
    auto selection = pfd::select_folder("Select save folder", ".").result();
    if (!selection.empty()) self -> setSavesPathInput(selection);
  }, this);

  scoutDirectoryButton->callback([](Fl_Widget*, void *v) {
    auto* self = static_cast<MainWindow*>(v);
    if (self->mainPresenter) self->mainPresenter->onScoutDirectoryClicked();
  }, this);

  syncButton->callback([](Fl_Widget*, void *v) {
    auto* self = static_cast<MainWindow*>(v);
    if (self->mainPresenter) self->mainPresenter->onSyncClicked();
  }, this);

  uploadButton->callback([](Fl_Widget*, void *v) {
    auto* self = static_cast<MainWindow*>(v);
    if (self->mainPresenter) self->mainPresenter->onUploadClicked();
  }, this);

  window->end();
};

MainWindow::~MainWindow() {
  delete window;
}

void MainWindow::setServerUrl(const std::string& url) {
  urlLabel->copy_label(("Connected to: " + url).c_str());
}

void MainWindow::setWorldChoices(const std::vector<std::string> &worldNames) {
  worldSelectChoice->clear();

  for (const auto& name : worldNames) {
    worldSelectChoice->add(name.c_str());
  }

  if (!worldNames.empty()) {
    worldSelectChoice->value(0);
  }

  worldSelectChoice->redraw();
  if (worldSelectChoice->window()) {
    worldSelectChoice->window()->redraw();
  }
}

void MainWindow::setStatus(const std::string& message, StatusType type, float progress) { // NOLINT(readability-make-member-function-const)
  statusProgress->copy_label(message.c_str());
  statusProgress->value(progress);

  using Type = StatusType;
  statusProgress->color2(type == Type::Error   ? FL_RED :
                        type == Type::Warning ? FL_YELLOW :
                        type == Type::Success ? FL_GREEN : FL_BLUE);
  statusProgress->redraw();
  Fl::check();
}

std::string MainWindow::getSavesPathInput() const {
  return savesPathInput->value();
}

void MainWindow::setSavesPathInput(const std::string& path) {
  savesPathInput->value(path.c_str());
}

int MainWindow::getSelectedWorldIndex() const {
  return worldSelectChoice->value();
}