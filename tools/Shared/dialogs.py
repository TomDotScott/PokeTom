"""
shared/dialogs.py
-----------------
Reusable Tkinter modal dialogs shared by all editor tools.

Callers create the dialog, call ``parent.wait_window(dialog)`` to block,
then read ``dialog.result``.  ``result`` is ``None`` if the user cancelled.
"""

from __future__ import annotations

import tkinter as tk
from tkinter import ttk


# ── Base dialog ───────────────────────────────────────────────────────────────

class BaseDialog(tk.Toplevel):
    """
    Shared scaffolding for all modal dialogs.

    Subclasses must override :meth:`_build_body` to populate the dialog and
    must set ``self.result`` before calling :meth:`_ok`.
    """

    def __init__(self, parent: tk.Misc, title: str) -> None:
        super().__init__(parent)
        self.title(title)
        self.transient(parent)
        self.resizable(False, False)
        self.result: object = None

        body_frame = ttk.Frame(self, padding=10)
        body_frame.pack(fill=tk.BOTH, expand=True)
        self._build_body(body_frame)

        btn_frame = ttk.Frame(self, padding=(10, 0, 10, 10))
        btn_frame.pack(fill=tk.X)
        self._build_buttons(btn_frame)

        self.bind("<Escape>", lambda _e: self._cancel())

        self.update_idletasks()
        px = parent.winfo_rootx() + parent.winfo_width() // 2 - self.winfo_width() // 2
        py = parent.winfo_rooty() + parent.winfo_height() // 2 - self.winfo_height() // 2
        self.geometry(f"+{max(0, px)}+{max(0, py)}")
        self.grab_set()

    def _build_body(self, frame: ttk.Frame) -> None:
        """Override to populate the dialog body."""

    def _build_buttons(self, frame: ttk.Frame) -> None:
        """Override to customise button row.  Default: OK + Cancel."""
        ttk.Button(frame, text="OK", command=self._ok, width=10).pack(side=tk.RIGHT, padx=(4, 0))
        ttk.Button(frame, text="Cancel", command=self._cancel, width=10).pack(side=tk.RIGHT)
        self.bind("<Return>", lambda _e: self._ok())

    def _ok(self) -> None:
        """Commit the result and close.  Override to validate first."""
        self.destroy()

    def _cancel(self) -> None:
        self.result = None
        self.destroy()


# ── Texture picker (from assets.yaml) ────────────────────────────────────────

class TexturePickerDialog(BaseDialog):
    """
    Let the user choose a texture from the list discovered in ``assets.yaml``.

    ``result`` is the chosen texture dict (``{'name': ..., 'source': ...}``)
    or ``None`` if cancelled.
    """

    def __init__(self, parent: tk.Misc, textures: list[dict]) -> None:
        self._textures = textures
        self._listbox: tk.Listbox
        super().__init__(parent, "Choose Texture")

    def _build_body(self, frame: ttk.Frame) -> None:
        ttk.Label(frame, text="Select a texture from assets.yaml:").pack(anchor=tk.W)
        lb_frame = ttk.Frame(frame)
        lb_frame.pack(fill=tk.BOTH, expand=True, pady=(4, 0))
        scrollbar = ttk.Scrollbar(lb_frame, orient=tk.VERTICAL)
        self._listbox = tk.Listbox(
            lb_frame, yscrollcommand=scrollbar.set,
            width=50, height=10, selectmode=tk.SINGLE,
        )
        scrollbar.configure(command=self._listbox.yview)
        self._listbox.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)

        for tex in self._textures:
            self._listbox.insert(tk.END, f"{tex.get('name', '?')}  →  {tex.get('source', '?')}")

        if self._textures:
            self._listbox.selection_set(0)
        self._listbox.bind("<Double-Button-1>", lambda _e: self._ok())

    def _ok(self) -> None:
        sel = self._listbox.curselection()
        if sel:
            self.result = self._textures[sel[0]]
        self.destroy()
