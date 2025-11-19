# Quick Reference: Using Images in Your GitHub README

## Upload Images to GitHub

1. **Commit the images:**
   ```bash
   git add ~Documentation/Images/
   git commit -m "Add promotional images"
   git push
   ```

2. **Or use GitHub web interface:**
   - Navigate to `~Documentation/Images/Features/`
   - Click "Add file" → "Upload files"
   - Drag and drop your images

## Reference Images in README.md

### Absolute GitHub URLs (Recommended for main README)

```markdown
![Layout Designer](https://raw.githubusercontent.com/YOUR-USERNAME/ViewportManager/main/~Documentation/Images/Features/layout-designer.png)
```

### Relative Paths (Works in repo)

```markdown
![Layout Designer](~Documentation/Images/Features/layout-designer.png)
```

## Example README Section with Images

```markdown
## Features

### Viewport Layout Designer

![Layout Designer Interface](~Documentation/Images/Features/layout-designer.png)

**Key Features:**
✓ Drag & drop visual editor with live preview
✓ 1-32 viewports with pixel-perfect positioning
✓ Custom camera & HUD assignment per viewport
✓ Save as reusable .uasset files
✓ Zero code required - setup in under 60 seconds

---

### Quick Start Wizard

![Quick Start Wizard](~Documentation/Images/Screenshots/quick-start-wizard.png)

One-click configuration with automatic validation:
- ✅ Game Viewport Client
- ✅ Game Instance & Game Mode
- ✅ Camera Input Mappings
- ✅ Max Splitscreen Players

---

### Example Layouts

<div align="center">

![Security Camera Grid](~Documentation/Images/Screenshots/security-camera-demo.png)
*4-way security camera surveillance system*

![Picture in Picture](~Documentation/Images/Screenshots/picture-in-picture.png)
*Picture-in-picture layout example*

</div>
```

## Image Size Optimization Tips

- Use PNG for UI/screenshots with transparency
- Use JPG for photos/gameplay (smaller size)
- Compress large images before upload
- Keep individual images under 1MB when possible
- Use GIF sparingly (large file sizes)

## Suggested Images to Create

Place these in the appropriate folders:

### Features/ (Feature Showcases)
- `layout-designer.png` - Main designer interface
- `quick-start-wizard.png` - Setup wizard with checkmarks
- `32-viewport-showcase.png` - All 32 viewports in action

### Screenshots/ (Real Usage)
- `security-camera-grid.png` - 4-way surveillance
- `picture-in-picture.png` - PIP demo
- `custom-layout.png` - Unique configuration

### UI/ (Widget Examples)
- `control-panel-widget.png` - Your control panel HUD
- `viewport-hud-example.png` - Per-viewport HUD

### Diagrams/ (Technical)
- `architecture-overview.png` - System architecture
- `coordinate-system.png` - Normalized coords explained
