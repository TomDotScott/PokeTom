"""
font_tool/main.py
-----------------
Entry point for the Bitmap Font Editor.

Run with:
    python main.py
or:
    cd tools/FontEditor && python main.py
"""

import sys
import os

# Ensure the shared/ sibling directory is on the path.
_HERE = os.path.dirname(os.path.abspath(__file__))
_TOOLS_ROOT = os.path.join(_HERE, "..")
_SHARED = os.path.join(_TOOLS_ROOT, "shared")

for _p in (_HERE, _SHARED):
    if _p not in sys.path:
        sys.path.insert(0, _p)

try:
    from PIL import Image  # noqa: F401 — early check
except ImportError:
    print(
        "ERROR: Pillow is required.  Install it with:\n"
        "    pip install Pillow\n"
        "or:\n"
        "    pip install -r requirements.txt",
        file=sys.stderr,
    )
    sys.exit(1)

from gui.app import App  # type: ignore[import]


def main() -> None:
    app = App()
    app.protocol("WM_DELETE_WINDOW", app._cmd_quit)
    app.mainloop()


if __name__ == "__main__":
    main()
