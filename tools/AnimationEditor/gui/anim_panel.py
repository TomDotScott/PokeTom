"""
gui/anim_panel.py
-----------------
Right-hand panel: animation list, frame editor, and embedded playback preview.

Layout (top to bottom)
----------------------
  Animations LabelFrame  -- listbox + New/Edit/Delete
  Info label             -- anchor, looping summary
  Frames LabelFrame      -- treeview + reorder/copy/paste/edit/delete
  Preview LabelFrame     -- canvas showing animated playback + Play/Stop button
"""

from __future__ import annotations
import tkinter as tk
from tkinter import ttk, messagebox
from typing import Callable, Optional

from PIL import Image, ImageTk

from models import Animation, Frame, AnimDict
from config import DEFAULT_DURATION_MS
from gui.dialogs import NewAnimationDialog, EditAnimationDialog, EditFrameDialog

# Zoom applied to the playback preview canvas
_PREVIEW_ZOOM = 3


class AnimPanel(ttk.Frame):
    """
    Displays and edits animations and their frames, with an embedded
    looping playback preview at the bottom.

    Callbacks
    ---------
    on_animation_selected(anim: Optional[Animation]) -> None
    on_dict_changed() -> None
    get_image_crop(x, y, w, h) -> Optional[Image.Image]
        Called by the preview to fetch a sprite crop from the full sheet.
    """

    def __init__(
        self,
        parent: tk.Widget,
        on_animation_selected: Callable[[Optional[Animation]], None],
        on_dict_changed: Callable[[], None],
        get_image_crop: Callable[[int, int, int, int], Optional[Image.Image]],
    ) -> None:
        super().__init__(parent)

        self._on_animation_selected = on_animation_selected
        self._on_dict_changed       = on_dict_changed
        self._get_image_crop        = get_image_crop

        self._anim_dict: Optional[AnimDict]  = None
        self._current_anim: Optional[Animation] = None
        self._clipboard: Optional[Frame]     = None

        # Playback state
        self._playing: bool                          = False
        self._play_frame_idx: int                    = 0
        self._play_after_id: Optional[str]           = None
        self._preview_tk: Optional[ImageTk.PhotoImage] = None

        self._build()

    # ------------------------------------------------------------------
    # Build
    # ------------------------------------------------------------------

    def _build(self) -> None:
        self.rowconfigure(0, weight=2)   # animation list
        self.rowconfigure(2, weight=3)   # frame list
        self.rowconfigure(4, weight=2)   # preview
        self.columnconfigure(0, weight=1)

        # ---- Animation list ----
        anim_frame = ttk.LabelFrame(self, text="Animations")
        anim_frame.grid(row=0, column=0, sticky="nsew", padx=4, pady=4)
        anim_frame.rowconfigure(0, weight=1)
        anim_frame.columnconfigure(0, weight=1)

        self._anim_listbox = tk.Listbox(
            anim_frame, selectmode="single", exportselection=False)
        self._anim_listbox.grid(row=0, column=0, sticky="nsew")
        sb = ttk.Scrollbar(anim_frame, orient="vertical",
                           command=self._anim_listbox.yview)
        sb.grid(row=0, column=1, sticky="ns")
        self._anim_listbox.configure(yscrollcommand=sb.set)
        self._anim_listbox.bind("<<ListboxSelect>>", self._on_anim_select)

        anim_btns = ttk.Frame(anim_frame)
        anim_btns.grid(row=1, column=0, columnspan=2, sticky="ew", pady=2)
        ttk.Button(anim_btns, text="+ New",  command=self._new_anim).pack(
            side="left", padx=2)
        ttk.Button(anim_btns, text="Edit",   command=self._edit_anim).pack(
            side="left", padx=2)
        ttk.Button(anim_btns, text="Delete", command=self._delete_anim).pack(
            side="left", padx=2)

        # ---- Info label ----
        self._anim_info_var = tk.StringVar()
        ttk.Label(self, textvariable=self._anim_info_var,
                  foreground="#888888").grid(
            row=1, column=0, sticky="w", padx=6)

        # ---- Frame list ----
        frame_frame = ttk.LabelFrame(self, text="Frames")
        frame_frame.grid(row=2, column=0, sticky="nsew", padx=4, pady=4)
        frame_frame.rowconfigure(0, weight=1)
        frame_frame.columnconfigure(0, weight=1)

        cols = ("idx", "x", "y", "w", "h", "dur", "extra")
        self._frame_tree = ttk.Treeview(
            frame_frame, columns=cols, show="headings", selectmode="browse")
        for col, head, width in [
            ("idx",   "#",     30),
            ("x",     "X",     45),
            ("y",     "Y",     45),
            ("w",     "W",     40),
            ("h",     "H",     40),
            ("dur",   "ms",    45),
            ("extra", "Extra", 100),
        ]:
            self._frame_tree.heading(col, text=head)
            self._frame_tree.column(col, width=width, anchor="center")

        frame_sb = ttk.Scrollbar(frame_frame, orient="vertical",
                                 command=self._frame_tree.yview)
        self._frame_tree.configure(yscrollcommand=frame_sb.set)
        self._frame_tree.grid(row=0, column=0, sticky="nsew")
        frame_sb.grid(row=0, column=1, sticky="ns")
        self._frame_tree.bind("<Double-Button-1>", self._on_frame_double_click)

        frame_btns = ttk.Frame(frame_frame)
        frame_btns.grid(row=1, column=0, columnspan=2, sticky="ew", pady=2)
        for label, cmd in [
            ("Up",     self._move_frame_up),
            ("Down",   self._move_frame_down),
            ("Copy",   self._copy_frame),
            ("Paste",  self._paste_frame),
            ("Delete", self._delete_frame),
            ("Edit…",  self._edit_frame),
        ]:
            ttk.Button(frame_btns, text=label, command=cmd).pack(
                side="left", padx=2)

        ttk.Separator(self, orient="horizontal").grid(
            row=3, column=0, sticky="ew", padx=4)

        # ---- Playback preview ----
        preview_frame = ttk.LabelFrame(self, text="Preview")
        preview_frame.grid(row=4, column=0, sticky="nsew", padx=4, pady=4)
        preview_frame.columnconfigure(0, weight=1)
        preview_frame.rowconfigure(0, weight=1)

        self._preview_canvas = tk.Canvas(
            preview_frame, bg="#1e1e1e", width=128, height=128)
        self._preview_canvas.grid(row=0, column=0, pady=4)

        self._play_btn = ttk.Button(
            preview_frame, text="Play", command=self._toggle_playback)
        self._play_btn.grid(row=1, column=0, pady=(0, 4))

    # ------------------------------------------------------------------
    # Public API
    # ------------------------------------------------------------------

    def set_anim_dict(self, anim_dict: AnimDict) -> None:
        self._stop_playback()
        self._anim_dict    = anim_dict
        self._current_anim = None
        self._refresh_anim_list()

    def add_frame_to_current(self, frame: Frame) -> None:
        """Append a frame to the currently selected animation."""
        if self._current_anim is None:
            messagebox.showwarning(
                "No animation",
                "Select or create an animation first.",
            )
            return
        self._current_anim.frames.append(frame)
        self._refresh_frame_list()
        self._on_dict_changed()

    def get_current_animation(self) -> Optional[Animation]:
        return self._current_anim

    # ------------------------------------------------------------------
    # Animation list
    # ------------------------------------------------------------------

    def _refresh_anim_list(self) -> None:
        self._anim_listbox.delete(0, "end")
        if self._anim_dict:
            for anim in self._anim_dict.animations:
                self._anim_listbox.insert("end", anim.name)
        self._refresh_frame_list()

    def _on_anim_select(self, _event: tk.Event) -> None:
        sel = self._anim_listbox.curselection()
        if not sel or not self._anim_dict:
            return
        self._stop_playback()
        self._current_anim = self._anim_dict.animations[sel[0]]
        self._update_anim_info()
        self._refresh_frame_list()
        self._on_animation_selected(self._current_anim)

    def _update_anim_info(self) -> None:
        if not self._current_anim:
            self._anim_info_var.set("")
            return
        a = self._current_anim
        loop_str = "looping" if a.looping else f"once → {a.on_end or '?'}"
        self._anim_info_var.set(f"anchor={int(a.anchor)}  {loop_str}")

    def _new_anim(self) -> None:
        if not self._anim_dict:
            return
        dlg = NewAnimationDialog(self, self._anim_dict.animation_names())
        if not dlg.result:
            return
        self._anim_dict.add_animation(dlg.result)
        self._refresh_anim_list()
        idx = len(self._anim_dict.animations) - 1
        self._anim_listbox.selection_set(idx)
        self._anim_listbox.see(idx)
        self._current_anim = dlg.result
        self._update_anim_info()
        self._on_animation_selected(self._current_anim)
        self._on_dict_changed()

    def _edit_anim(self) -> None:
        if not self._current_anim or not self._anim_dict:
            return
        old_name = self._current_anim.name
        dlg = EditAnimationDialog(
            self, self._current_anim, self._anim_dict.animation_names())
        if not dlg.result:
            return
        r = dlg.result
        if r["name"] != old_name:
            self._anim_dict.rename_animation(old_name, r["name"])
        self._current_anim.looping = r["looping"]
        self._current_anim.anchor  = r["anchor"]
        self._current_anim.on_end  = r["on_end"]
        self._refresh_anim_list()
        self._update_anim_info()
        self._on_dict_changed()

    def _delete_anim(self) -> None:
        if not self._current_anim or not self._anim_dict:
            return
        if not messagebox.askyesno("Delete",
                                   f"Delete '{self._current_anim.name}'?"):
            return
        self._stop_playback()
        self._anim_dict.remove_animation(self._current_anim.name)
        self._current_anim = None
        self._refresh_anim_list()
        self._on_animation_selected(None)
        self._on_dict_changed()

    # ------------------------------------------------------------------
    # Frame list
    # ------------------------------------------------------------------

    def _refresh_frame_list(self) -> None:
        self._frame_tree.delete(*self._frame_tree.get_children())
        if not self._current_anim:
            return
        for i, f in enumerate(self._current_anim.frames):
            extra = ", ".join(f"{k}={v}" for k, v in f.extra_attrs.items())
            self._frame_tree.insert("", "end", iid=str(i), values=(
                i, f.top_left_x, f.top_left_y,
                f.sprite_width, f.sprite_height,
                f.duration, extra,
            ))

    def _selected_frame_index(self) -> Optional[int]:
        sel = self._frame_tree.selection()
        return int(sel[0]) if sel else None

    def _on_frame_double_click(self, _event: tk.Event) -> None:
        self._edit_frame()

    def _edit_frame(self) -> None:
        idx = self._selected_frame_index()
        if idx is None or not self._current_anim:
            return
        frame = self._current_anim.frames[idx]
        dlg = EditFrameDialog(self, frame)
        if dlg.result:
            frame.duration    = dlg.result["duration"]
            frame.extra_attrs = dlg.result["extra_attrs"]
            self._refresh_frame_list()
            self._on_dict_changed()

    def _move_frame_up(self) -> None:
        idx = self._selected_frame_index()
        if idx is None or idx == 0 or not self._current_anim:
            return
        f = self._current_anim.frames
        f[idx - 1], f[idx] = f[idx], f[idx - 1]
        self._refresh_frame_list()
        self._frame_tree.selection_set(str(idx - 1))
        self._on_dict_changed()

    def _move_frame_down(self) -> None:
        idx = self._selected_frame_index()
        if not self._current_anim:
            return
        f = self._current_anim.frames
        if idx is None or idx >= len(f) - 1:
            return
        f[idx], f[idx + 1] = f[idx + 1], f[idx]
        self._refresh_frame_list()
        self._frame_tree.selection_set(str(idx + 1))
        self._on_dict_changed()

    def _copy_frame(self) -> None:
        idx = self._selected_frame_index()
        if idx is None or not self._current_anim:
            return
        self._clipboard = self._current_anim.frames[idx].copy()

    def _paste_frame(self) -> None:
        if self._clipboard is None or not self._current_anim:
            return
        idx       = self._selected_frame_index()
        insert_at = (idx + 1) if idx is not None else len(
            self._current_anim.frames)
        self._current_anim.frames.insert(insert_at, self._clipboard.copy())
        self._refresh_frame_list()
        self._on_dict_changed()

    def _delete_frame(self) -> None:
        idx = self._selected_frame_index()
        if idx is None or not self._current_anim:
            return
        del self._current_anim.frames[idx]
        self._refresh_frame_list()
        self._on_dict_changed()

    # ------------------------------------------------------------------
    # Playback
    # ------------------------------------------------------------------

    def _toggle_playback(self) -> None:
        if self._playing:
            self._stop_playback()
        else:
            self._start_playback()

    def _start_playback(self) -> None:
        if not self._current_anim or not self._current_anim.frames:
            return
        self._playing        = True
        self._play_frame_idx = 0
        self._play_btn.configure(text="Stop")
        self._tick_playback()

    def _stop_playback(self) -> None:
        self._playing = False
        if self._play_after_id is not None:
            self.after_cancel(self._play_after_id)
            self._play_after_id = None
        self._play_btn.configure(text="Play")
        self._preview_canvas.delete("all")
        self._preview_tk = None

    def _tick_playback(self) -> None:
        """Display the current frame then schedule the next tick."""
        if not self._playing or not self._current_anim:
            return

        frames = self._current_anim.frames
        if not frames:
            self._stop_playback()
            return

        frame = frames[self._play_frame_idx]
        self._render_preview_frame(frame)

        # Advance index, looping unconditionally
        self._play_frame_idx = (self._play_frame_idx + 1) % len(frames)
        self._play_after_id  = self.after(
            frame.duration, self._tick_playback)

    def _render_preview_frame(self, frame: Frame) -> None:
        """Crop the sprite from the sheet and draw it on the preview canvas."""
        crop = self._get_image_crop(
            frame.top_left_x,
            frame.top_left_y,
            frame.sprite_width,
            frame.sprite_height,
        )
        if crop is None:
            return

        z      = _PREVIEW_ZOOM
        disp_w = max(1, crop.width  * z)
        disp_h = max(1, crop.height * z)
        scaled = crop.resize((disp_w, disp_h), Image.NEAREST)

        # Size canvas to fit the sprite
        self._preview_canvas.configure(width=disp_w, height=disp_h)
        self._preview_tk = ImageTk.PhotoImage(scaled)
        self._preview_canvas.delete("all")
        self._preview_canvas.create_image(
            0, 0, anchor="nw", image=self._preview_tk)
