# discord-rm

**discord-rm** is a command-line tool designed to remove messages from Discord channels using an authorization token and the Discord API.

---

## ⚠️ Warning

Using self-bots violates Discord's Terms of Service and may result in account termination. Proceed with caution.

--- 

## 🛠️ Options

This tool utilizes [ArgParse](https://github.com/p-ranav/argparse) for command-line argument parsing.

| Short | Full            | Description                                                                                |
| ----- | --------------- | ------------------------------------------------------------------------------------------ | 
| `-v`  | `--verbose`     | Enables verbose logging.                                                                   |                                     
| `-d`  | `--debug`       | Outputs debug information for developers.                                                  |                                           
| `-nc` | `--no-confirm`  | Deletes messages without initial confirmation.                                             |                                      
| `-i`  | `--interactive` | Prompts for required data interactively instead of via command-line arguments.             | 
| `-sif`| `--skip-if-fail`| Won't exit if message deletion fails.                                                      | 
| `-s`  | `--sender-id`   | Specifies the user ID whose messages should be removed (requires appropriate permissions). |                                   
| `-g`  | `--guild-id`    | Specifies the server (guild) ID where messages should be removed.                          |                    
| `-c`  | `--channel-id`  | Specifies the channel ID within the guild where messages should be removed.                | 

---

## 📦 Dependencies

* [p-ranav/ArgParse](https://github.com/p-ranav/argparse)
* [nlohmann/json](https://github.com/nlohmann/json)
* [curl/curl](https://github.com/curl/curl)
* [fmtlib/fmt](https://github.com/fmtlib/fmt)

---

## 🧱 Build Instructions

To build the project, follow these steps:

```bash
git clone https://github.com/beliumgl/discord-rm
cd discord-rm
mkdir build && cd build
cmake ..
cmake --build .
```



---

## 💻 Platform Compatibility

The tool was developed on Linux but is compatible with Windows and macOS, as all dependencies are cross-platform.

---
