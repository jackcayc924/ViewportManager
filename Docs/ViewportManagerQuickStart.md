# Viewport Manager Quick Start

Follow the checklist below when bringing the Viewport Manager plugin into a fresh project.

## 1. Enable the plugin
- Copy the `ViewportManager` folder into your project's `Plugins/` directory.
- Launch the project and enable **Viewport Manager** in *Edit → Plugins*.
- Restart the editor when prompted.

## 2. Place the auto layout actor
- Open the level you want to use for split-screen testing.
- Run **Window → Viewport Manager → Quick Start Wizard** and press **Add Auto Layout Actor**.
- The wizard selects the new `AVMAutoLayoutActor`; move it where convenient (location is irrelevant in PIE).

## 3. Assign a layout asset
- In the wizard, click **Assign VM_DemoLayout** to link the sample layout.
- Alternatively, use the **Layout Designer** window to build your own `UVMSplitLayoutAsset` and assign it in the actor's details.

## 4. Review project settings
- Click **Open Project Settings** in the wizard (or navigate to *Project Settings → Plugins → Viewport Manager*).
- Set **Default Layout** to your layout asset.
- Leave **Auto Add Missing Local Players** enabled while testing to ensure pawns spawn automatically.

## 5. Explore the demo map
- Use **Open Demo Map** to load `DemoViewports.umap`.
- Press **Play** in PIE. Up to four panes should appear, each with its own camera mode:
  - Orbit cameras default to the enhanced orbit/pan/zoom controls.
  - Free-cam panes use the new WASD + mouse fly mode (hold `Shift` to sprint).

## 6. Edit layouts visually
- Open **Window → Viewport Manager → Layout Designer**.
- Select a `UVMSplitLayoutAsset` from the picker at the top.
- Click panes in the live preview to edit their properties:
  - Choose the camera mode (Orbit, Free, or Custom pawn).
  - Adjust per-pane input presets: sensitivity, axis inversion, pan locks, zoom scaling.
  - Use the Snap buttons (50 %, 25 %, 10 %) to align origins and sizes precisely.
- Changes hot-reload into PIE; keep the designer open during iteration.

## 7. Validate runtime behaviour
- In PIE, confirm that keyboard/mouse input follows the hovered pane and that left-click focuses it.
- Verify HUD widgets stay clipped to their panes. If you supply your own HUD, derive from `UVMViewportHUDWidget` to inherit helper utilities.
- Test both camera behaviours:
  - **Orbit**: Right mouse to orbit, Middle mouse to pan, Mouse wheel to zoom. Inversion and lock settings mirror layout values.
  - **Free Cam**: WASD for planar movement, `Space`/`Ctrl` for vertical, `Shift` to sprint. Orbit toggles gate look input; pan toggles gate translation.

## 8. Package-ready checklist
- Remove the demo actor before shipping and replace it with your own layout assignments.
- Create dedicated layout assets per game mode rather than editing the demo asset in-place.
- Clear the `Content/Demo` reference if you do not intend to ship the sample map.

## Troubleshooting
- **No panes appear**: ensure the project is using the custom `UVMGameViewportClient` (automatic when the plugin is enabled) and that the auto layout actor references a layout.
- **Mouse cursor disappears**: per-pane cursor visibility is now configured in the layout asset. Set `bKeepMouseCursorVisible` for panes that should retain UI focus.
- **Free camera not responding**: confirm the pane's camera mode is set to *Free* and that `bPanEnabled` + `bCameraControlsEnabled` remain true.
- **HUD spans entire screen**: register the HUD class in the layout asset; the viewport client anchors it automatically when the class derives from `UVMViewportHUDWidget`.

Keep this guide alongside your project documentation so onboarding new team members is frictionless.
