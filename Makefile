SERVER_BINARY   := minesync_server
CLIENT_BINARY   := minesync_client
SERVICE_NAME    := minesync-server
BUILD_DIR       := build
PREFIX          ?= /usr/local
BINDIR          := $(PREFIX)/bin
SYSTEMD_DIR     := /etc/systemd/system

SERVICE_USER    ?= $(shell echo $${SUDO_USER:-$$USER})

ifeq ($(OS),Windows_NT)
    IS_WINDOWS := 1
    RM := del /q /f
    RMDIR := rmdir /s /q
else
    IS_WINDOWS := 0
    RM := rm -f
    RMDIR := rm -rf
endif

.PHONY: all build build-server build-client install-server uninstall-server status clean help

all: build

$(BUILD_DIR)/build.ninja:
	@echo "==> Configuring CMake with Ninja..."
	cmake -B $(BUILD_DIR) -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$(PREFIX)

build: $(BUILD_DIR)/build.ninja
	@echo "==> Building MineSync (Client & Server)..."
	cmake --build $(BUILD_DIR) --config Release -j$$(nproc)

build-server: $(BUILD_DIR)/build.ninja
	@echo "==> Building MineSync Server..."
	cmake --build $(BUILD_DIR) --target $(SERVER_BINARY) --config Release -j$$(nproc)

build-client: $(BUILD_DIR)/build.ninja
	@echo "==> Building MineSync Client..."
	cmake --build $(BUILD_DIR) --target $(CLIENT_BINARY) --config Release -j$$(nproc)

install-server:
ifeq ($(IS_WINDOWS),1)
	@echo "Error: systemd service installation is only supported on Linux."
	@echo "On Windows, consider using NSSM (Non-Sucking Service Manager) to run minesync_server as a service."
	@exit 1
else
	@$(MAKE) build-server
	@if [ "$$(id -u)" -ne 0 ]; then \
		echo "Error: 'sudo make install-server' is required."; \
		exit 1; \
	fi
	@echo "==> Installing $(SERVER_BINARY) to $(DESTDIR)$(BINDIR)..."
	cmake --install $(BUILD_DIR) --component server --prefix $(DESTDIR)$(PREFIX)
	@echo "==> Creating systemd service file..."
	@printf "[Unit]\n\
Description=MineSync Server\n\
After=network.target\n\n\
[Service]\n\
Type=simple\n\
User=$(SERVICE_USER)\n\
WorkingDirectory=/home/$(SERVICE_USER)\n\
ExecStart=$(BINDIR)/$(SERVER_BINARY)\n\
Restart=on-failure\n\
RestartSec=5\n\n\
[Install]\n\
WantedBy=multi-user.target\n" > $(DESTDIR)$(SYSTEMD_DIR)/$(SERVICE_NAME).service
	@if [ -z "$(DESTDIR)" ]; then \
		echo "==> Reloading systemd and enabling service..."; \
		systemctl daemon-reload; \
		systemctl enable $(SERVICE_NAME); \
		systemctl restart $(SERVICE_NAME); \
		echo ""; \
		echo "Done! MineSync Server is active as '$(SERVICE_NAME)'."; \
		echo "  Check status: make status"; \
		echo "  View logs:   sudo journalctl -u $(SERVICE_NAME) -f"; \
	fi
endif

uninstall-server:
	@if [ "$$(id -u)" -ne 0 ]; then \
		echo "Error: 'sudo make uninstall-server' is required."; \
		exit 1; \
	fi
	@echo "==> Stopping and disabling $(SERVICE_NAME)..."
	-systemctl stop $(SERVICE_NAME) 2>/dev/null
	-systemctl disable $(SERVICE_NAME) 2>/dev/null
	@echo "==> Removing installed binary and systemd service file..."
	rm -f $(DESTDIR)$(BINDIR)/$(SERVER_BINARY)
	rm -f $(DESTDIR)$(SYSTEMD_DIR)/$(SERVICE_NAME).service
	@if [ -z "$(DESTDIR)" ]; then \
		systemctl daemon-reload; \
		echo "Uninstalled $(SERVICE_NAME) successfully."; \
	fi

status:
	@systemctl status $(SERVICE_NAME)

clean:
	@echo "==> Removing build directory..."
	cmake --build build --target clean 2>/dev/null || $(RMDIR) build

help:
	@echo "MineSync Build & Management Commands:"
	@echo "  make build           Build both client and server targets"
	@echo "  make build-server    Build server executable only"
	@echo "  make build-client    Build client executable only"
	@echo "  sudo make install-server   Build, install server binary, and launch systemd service"
	@echo "  sudo make uninstall-server Stop server service and remove binary"
	@echo "  make status          Show current status of systemd server service"
	@echo "  make clean           Delete the cmake build directory"
