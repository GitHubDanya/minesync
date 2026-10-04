#include "ConnectionWindow.h"

#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>

#include "presenters/ConnectionPresenter.h"

ConnectionWindow::ConnectionWindow() {
    window = new Fl_Window(640, 120, "Specify a Connection");

    Fl_Box *title = new Fl_Box(20, 0, 600, 50, "Enter the root url of your minesync server: (e.g. 192.168.0.1:1234)");
    urlPrefix = new Fl_Box(20, 50, 40, 20);
    connectionInput = new Fl_Input(65, 50, 555, 20);
    connectButton = new Fl_Button(20, 80, 600, 20, "Connect");

    connectButton->callback([](Fl_Widget*, void*v) {
        auto* self = static_cast<ConnectionWindow*>(v);
        if (self->connectionPresenter) self->connectionPresenter->onConnectButtonClicked();
    }, this);

    window->end();
}

ConnectionWindow::~ConnectionWindow() {
    delete window;
}

void ConnectionWindow::setUrlPrefix(const std::string label) {
    urlPrefix->copy_label(label.c_str());
}

std::string ConnectionWindow::getUrlInput() const {
    return connectionInput->value();
}

void ConnectionWindow::setButtonLabel(const std::string label) {
    connectButton->copy_label(label.c_str());
}
