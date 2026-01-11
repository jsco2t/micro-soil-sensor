# Makefile for ESP32 Soil Sensor PlatformIO Project
# Provides user-friendly wrapper around PlatformIO commands

# Configuration
ENV_C6 := sparkfun_esp32c6_thing_plus
ENV_S3 := sparkfun_esp32s3_thing_plus
MONITOR_BAUD := 115200
MONITOR_FILTER := direct

# Python and virtual environment configuration
PYTHON := python3.13
VENV_DIR := .venv
VENV_BIN := $(VENV_DIR)/bin
PIO := $(VENV_BIN)/pio
PYTHON_VENV := $(VENV_BIN)/python

# Color output
GREEN := \033[0;32m
YELLOW := \033[1;33m
RED := \033[0;31m
NC := \033[0m

# Phony targets (not actual files)
.PHONY: help setup build build-c6 build-s3 upload upload-c6 upload-s3 monitor clean
.PHONY: check-env check-secrets info rebuild rebuild-c6 rebuild-s3 size

# Default target
.DEFAULT_GOAL := help

help: ## Show this help message
	@echo "ESP32 Soil Sensor - Available Make Targets"
	@echo "==========================================="
	@echo ""
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | \
		awk 'BEGIN {FS = ":.*?## "}; {printf "  $(GREEN)%-15s$(NC) %s\n", $$1, $$2}'
	@echo ""
	@echo "Quick Start (New Machine):"
	@printf "  1. Initialize environment: $(YELLOW)make setup$(NC)\n"
	@echo "  2. Copy secret files:"
	@printf "     $(YELLOW)cp include/wifi_secrets_example.h include/wifi_secrets.h$(NC)\n"
	@printf "     $(YELLOW)cp include/mqtt_secrets_example.h include/mqtt_secrets.h$(NC)\n"
	@printf "  3. Build firmware: $(YELLOW)make build$(NC)\n"
	@printf "  4. Upload to board: $(YELLOW)make upload$(NC)\n"
	@echo ""
	@echo "Note: You can also use direnv for automatic environment loading"
	@echo ""

setup: ## Initialize Python virtual environment and install PlatformIO
	@printf "$(GREEN)Setting up build environment...$(NC)\n"
	@# Check if python3.13 is available
	@if ! command -v $(PYTHON) >/dev/null 2>&1; then \
		printf "$(RED)ERROR: $(PYTHON) not found!$(NC)\n"; \
		printf "$(YELLOW)PlatformIO requires Python 3.10-3.13$(NC)\n"; \
		printf "$(YELLOW)Install it with: sudo dnf install python3.13$(NC)\n"; \
		exit 1; \
	fi
	@# Create virtual environment if it doesn't exist
	@if [ ! -d "$(VENV_DIR)" ]; then \
		printf "$(YELLOW)Creating Python virtual environment with $(PYTHON)...$(NC)\n"; \
		$(PYTHON) -m venv $(VENV_DIR); \
		printf "$(YELLOW)Installing PlatformIO and dependencies...$(NC)\n"; \
		$(VENV_BIN)/pip install --upgrade pip setuptools wheel --quiet; \
		$(VENV_BIN)/pip install platformio pyyaml --quiet; \
		printf "$(GREEN)✓ Build environment created successfully$(NC)\n"; \
	else \
		printf "$(YELLOW)Virtual environment already exists$(NC)\n"; \
		if [ ! -f "$(PIO)" ]; then \
			printf "$(YELLOW)PlatformIO not found, installing...$(NC)\n"; \
			$(VENV_BIN)/pip install --upgrade pip setuptools wheel platformio pyyaml --quiet; \
		fi; \
		printf "$(GREEN)✓ Build environment ready$(NC)\n"; \
	fi
	@printf "$(GREEN)✓ Python: $$($(PYTHON_VENV) --version)$(NC)\n"
	@printf "$(GREEN)✓ PlatformIO: $$($(PIO) --version | head -n1)$(NC)\n"

check-env: ## Verify build environment is ready
	@# Check if .venv exists
	@if [ ! -d "$(VENV_DIR)" ]; then \
		printf "$(RED)ERROR: Virtual environment not found!$(NC)\n"; \
		printf "$(YELLOW)Run: make setup$(NC)\n"; \
		exit 1; \
	fi
	@# Check if PlatformIO is installed
	@if [ ! -f "$(PIO)" ]; then \
		printf "$(RED)ERROR: PlatformIO not found in $(VENV_DIR)!$(NC)\n"; \
		printf "$(YELLOW)Run: make setup$(NC)\n"; \
		exit 1; \
	fi
	@# Verify Python version is compatible
	@PYTHON_VERSION=$$($(PYTHON_VENV) -c 'import sys; print(".".join(map(str, sys.version_info[:2])))'); \
	if ! $(PYTHON_VENV) -c 'import sys; sys.exit(0 if (3, 10) <= sys.version_info[:2] <= (3, 13) else 1)' 2>/dev/null; then \
		printf "$(RED)ERROR: Python $$PYTHON_VERSION is not compatible!$(NC)\n"; \
		printf "$(YELLOW)PlatformIO requires Python 3.10-3.13$(NC)\n"; \
		printf "$(YELLOW)Remove $(VENV_DIR) and run: make setup$(NC)\n"; \
		exit 1; \
	fi
	@printf "$(GREEN)✓ Build environment ready (Python $$($(PYTHON_VENV) --version | cut -d' ' -f2))$(NC)\n"

check-secrets: ## Verify required secret files exist
	@echo "Checking for required secret files..."
	@if [ ! -f include/wifi_secrets.h ]; then \
		printf "$(RED)ERROR: include/wifi_secrets.h not found!$(NC)\n"; \
		printf "$(YELLOW)Please copy include/wifi_secrets_example.h to include/wifi_secrets.h$(NC)\n"; \
		printf "$(YELLOW)and update with your WiFi credentials.$(NC)\n"; \
		exit 1; \
	fi
	@if [ ! -f include/mqtt_secrets.h ]; then \
		printf "$(RED)ERROR: include/mqtt_secrets.h not found!$(NC)\n"; \
		printf "$(YELLOW)Please copy include/mqtt_secrets_example.h to include/mqtt_secrets.h$(NC)\n"; \
		printf "$(YELLOW)and update with your MQTT broker details.$(NC)\n"; \
		exit 1; \
	fi
	@printf "$(GREEN)✓ All secret files present$(NC)\n"

build-c6: check-env check-secrets ## Build firmware for ESP32-C6 board
	@printf "$(GREEN)Building for ESP32-C6...$(NC)\n"
	@$(PIO) run -e $(ENV_C6)

build-s3: check-env check-secrets ## Build firmware for ESP32-S3 board
	@printf "$(GREEN)Building for ESP32-S3...$(NC)\n"
	@$(PIO) run -e $(ENV_S3)

build: build-c6 build-s3 ## Build firmware for both boards (C6 and S3)
	@printf "$(GREEN)✓ All boards built successfully$(NC)\n"

upload-c6: build-c6 ## Build and upload firmware to ESP32-C6 board
	@printf "$(GREEN)Uploading to ESP32-C6...$(NC)\n"
	@$(PIO) run -t upload -e $(ENV_C6)

upload-s3: build-s3 ## Build and upload firmware to ESP32-S3 board
	@printf "$(GREEN)Uploading to ESP32-S3...$(NC)\n"
	@$(PIO) run -t upload -e $(ENV_S3)

upload: check-env ## Interactive upload - prompts for board selection
	@echo "Select board to upload:"
	@echo "  1) ESP32-C6 (sparkfun_esp32c6_thing_plus)"
	@echo "  2) ESP32-S3 (sparkfun_esp32s3_thing_plus)"
	@read -p "Enter choice [1-2]: " choice; \
	case $$choice in \
		1) $(MAKE) upload-c6 ;; \
		2) $(MAKE) upload-s3 ;; \
		*) printf "$(RED)Invalid choice$(NC)\n"; exit 1 ;; \
	esac

monitor: check-env ## Open serial monitor (115200 baud)
	@printf "$(GREEN)Opening serial monitor ($(MONITOR_BAUD) baud)...$(NC)\n"
	@printf "$(YELLOW)Press Ctrl+C to exit$(NC)\n"
	@$(PIO) device monitor --baud $(MONITOR_BAUD) --filter $(MONITOR_FILTER)

clean: check-env ## Clean build artifacts for all environments
	@printf "$(GREEN)Cleaning build artifacts...$(NC)\n"
	@$(PIO) run -t clean
	@printf "$(GREEN)✓ Clean complete$(NC)\n"

rebuild-c6: clean build-c6 ## Clean and rebuild ESP32-C6 firmware

rebuild-s3: clean build-s3 ## Clean and rebuild ESP32-S3 firmware

rebuild: clean build ## Clean and rebuild all boards

size: check-env ## Show firmware size for both boards
	@printf "$(GREEN)=== ESP32-C6 Firmware Size ===$(NC)\n"
	@$(PIO) run -e $(ENV_C6) -t size 2>/dev/null || printf "$(YELLOW)Build first: make build-c6$(NC)\n"
	@echo ""
	@printf "$(GREEN)=== ESP32-S3 Firmware Size ===$(NC)\n"
	@$(PIO) run -e $(ENV_S3) -t size 2>/dev/null || printf "$(YELLOW)Build first: make build-s3$(NC)\n"

info: ## Show project information and status
	@printf "$(GREEN)ESP32 Soil Sensor - Project Information$(NC)\n"
	@echo "========================================"
	@echo ""
	@printf "$(GREEN)Board Environments:$(NC)\n"
	@printf "  - $(YELLOW)$(ENV_C6)$(NC) (ESP32-C6)\n"
	@printf "  - $(YELLOW)$(ENV_S3)$(NC) (ESP32-S3)\n"
	@echo ""
	@printf "$(GREEN)Secret Files Status:$(NC)\n"
	@if [ -f include/wifi_secrets.h ]; then \
		printf "  $(GREEN)✓$(NC) wifi_secrets.h\n"; \
	else \
		printf "  $(RED)✗$(NC) wifi_secrets.h $(YELLOW)(missing)$(NC)\n"; \
	fi
	@if [ -f include/mqtt_secrets.h ]; then \
		printf "  $(GREEN)✓$(NC) mqtt_secrets.h\n"; \
	else \
		printf "  $(RED)✗$(NC) mqtt_secrets.h $(YELLOW)(missing)$(NC)\n"; \
	fi
	@echo ""
	@printf "$(GREEN)Build Environment:$(NC)\n"
	@if [ -d "$(VENV_DIR)" ]; then \
		if [ -f "$(PIO)" ]; then \
			printf "  Python venv: $(GREEN)active$(NC) ($(VENV_DIR)/)\n"; \
			printf "  Python: $$($(PYTHON_VENV) --version | cut -d' ' -f2)\n"; \
			printf "  PlatformIO: $$($(PIO) --version | head -n1)\n"; \
		else \
			printf "  Python venv: $(YELLOW)exists but PlatformIO not installed$(NC)\n"; \
			printf "  $(YELLOW)Run: make setup$(NC)\n"; \
		fi; \
	else \
		printf "  Python venv: $(RED)not created$(NC)\n"; \
		printf "  $(YELLOW)Run: make setup$(NC)\n"; \
	fi
	@echo ""
