"""
main.py
-------
Entry point for the AnimDict Editor.

Usage:
    python main.py
    python main.py path/to/assets.yaml
    python main.py path/to/spritesheet.png
"""

import sys
from pathlib import Path


def main() -> None:
    # Ensure our package root is on sys.path so sibling imports work
    sys.path.insert(0, str(Path(__file__).parent))

    from gui.app import App

    app = App()

    # If a file was passed on the command line, load it after the window opens
    if len(sys.argv) > 1:
        arg = Path(sys.argv[1])
        if arg.suffix.lower() in (".yaml", ".yml"):
            app.after(100, lambda: app._open_yaml_path(arg))
        elif arg.suffix.lower() in (".png", ".jpg", ".jpeg", ".bmp"):
            app.after(100, lambda: app._load_image(arg, arg.stem.upper()))

    app.mainloop()


if __name__ == "__main__":
    main()
