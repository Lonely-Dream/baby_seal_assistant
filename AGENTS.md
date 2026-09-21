---
name: "ESP-IDF ESP32-S3 Workflow"
description: "Use when working on this ESP-IDF ESP32-S3 firmware project, including configuring, building, flashing, monitoring, debugging, sizing, or running idf.py commands. Prefer ESP-IDF MCP operations and use the configured PowerShell fallback when MCP is unavailable or insufficient."
---

# ESP-IDF ESP32-S3 Workflow

- This workspace is an ESP-IDF firmware project targeting ESP32-S3. Preserve ESP32-S3 compatibility when making configuration or implementation changes.
- For ESP-IDF tasks, first use the ESP-IDF MCP tools when they can perform the requested operation.
- Use the PowerShell fallback only when the MCP tools are unavailable, return an error, or cannot complete the requested operation.
- Before using the fallback, activate the ESP-IDF environment in PowerShell from the workspace root:

  ```powershell
  . 'C:\Espressif\tools\Microsoft.v6.1.PowerShell_profile.ps1'
  ```

- After activation, run the appropriate `idf.py <command>` command, such as `idf.py build`, `idf.py flash`, or `idf.py monitor`.