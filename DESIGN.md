# Design System: LinuxChat Modern Gray
**Platform:** Qt6 Widgets (Desktop)
**Theme:** Light Modern Gray

---

## Configuration — Set Your Style

| Dial | Level | Description |
|------|-------|-------------|
| **Creativity** | `5` | Balanced, clean but with personality. Not too minimal, not too expressive. |
| **Density** | `5` | Balanced sections. Standard chat app spacing. |
| **Variance** | `4` | Subtle offsets. Clean, predictable layout with slight asymmetry. |
| **Motion Intent** | `4` | Subtle hover/entrance cues. Functional animations only. |

---

## 1. Visual Theme & Atmosphere
A clean, modern chat interface with a light gray canvas. The atmosphere is professional yet approachable - like a well-organized workspace where every element serves a purpose. Density is balanced, variance runs low for consistency, and motion is subtle and functional. The overall impression: clean, efficient, modern.

---

## 2. Color Palette & Roles

### Primary Palette
- **Canvas Gray** (#F5F5F5) — Primary background surface. Warm light gray, never clinical white.
- **Pure Surface** (#FFFFFF) — Card and container fill. Clean white for elevated content.
- **Charcoal Ink** (#1F2937) — Primary text. Near-black with warmth, never pure black.
- **Steel Secondary** (#6B7280) — Body text, descriptions, metadata. Gray-500 warmth.
- **Muted Slate** (#9CA3AF) — Tertiary text, timestamps, disabled states.
- **Whisper Border** (#E5E7EB) — Card borders, structural 1px lines. Light gray.
- **Diffused Shadow** (rgba(0,0,0,0.08)) — Card elevation. Soft, wide-spreading.

### Accent Selection
- **Electric Blue** (#3B82F6) — Primary accent for interactive elements, buttons, links.
- **Blue Hover** (#2563EB) — Hover state for accent elements.
- **Blue Pressed** (#1D4ED8) — Pressed state for accent elements.

### Semantic Colors
- **Success Green** (#10B981) — Online status, successful actions.
- **Warning Amber** (#F59E0B) — Warnings, attention needed.
- **Danger Red** (#EF4444) — Errors, destructive actions, offline status.

### Banned Colors
- Pure Black (#000000) — always use Charcoal Ink (#1F2937)
- Pure White (#FFFFFF) for backgrounds — always use Canvas Gray (#F5F5F5)
- Oversaturated accents above 80%
- Dark themes — this is a light-theme-only design system

---

## 3. Typography Rules

### Font Stack
- **Primary:** "LXGW WenKai" (霞鹜文楷), "Microsoft YaHei UI", "Segoe UI", sans-serif
- **Monospace:** "Cascadia Code", "Consolas", monospace
- **Embedded:** LXGW WenKai 字体文件已嵌入资源（`resources/fonts/`）

### Scale
- **Display/Headers:** 16-20px, weight 600, Charcoal Ink color
- **Body:** 14px, weight 400, Charcoal Ink color
- **Secondary/Meta:** 12px, weight 400, Steel Secondary color
- **Timestamps:** 11px, weight 400, Muted Slate color

### Line Height
- **Headers:** 1.3
- **Body:** 1.5
- **Compact lists:** 1.2

---

## 4. Component Stylings

### Buttons
- **Primary:** Electric Blue (#3B82F6) fill, white text, 8px radius, 10px 24px padding
- **Secondary:** Pure Surface (#FFFFFF) fill, Charcoal Ink text, 1px Whisper Border, 8px radius
- **Ghost:** Transparent fill, Electric Blue text, no border
- **Danger:** Transparent fill, Danger Red text, 1px Danger Red border
- **Hover:** Background shifts to Blue Hover (#2563EB) for primary, light gray for secondary
- **Pressed:** Background shifts to Blue Pressed (#1D4ED8), subtle translateY(-1px)
- **Disabled:** Gray background, Muted Slate text

### Cards/Containers
- **Background:** Pure Surface (#FFFFFF)
- **Border:** 1px Whisper Border (#E5E7EB)
- **Radius:** 8px (standard), 12px (large cards)
- **Shadow:** Diffused Shadow (0 2px 8px rgba(0,0,0,0.08))
- **Padding:** 16px (standard), 24px (large)

### Input Fields
- **Background:** Pure Surface (#FFFFFF)
- **Border:** 1px Whisper Border (#E5E7EB)
- **Radius:** 8px
- **Padding:** 10px 14px
- **Focus:** 2px Electric Blue border, no background change
- **Error:** 2px Danger Red border, light red background tint
- **Placeholder:** Muted Slate (#9CA3AF) color

### Message Bubbles
- **Self messages:** Electric Blue (#3B82F6) background, white text, 8px radius
- **Other messages:** Pure Surface (#FFFFFF) background, Charcoal Ink text, 1px Whisper Border
- **System messages:** Canvas Gray (#F5F5F5) background, Steel Secondary text, centered

### Sidebar
- **Background:** Pure Surface (#FFFFFF)
- **Border:** 1px Whisper Border on right side
- **Section headers:** 12px, weight 600, Steel Secondary color, uppercase tracking
- **User list items:** 32px height, 8px 12px padding, 6px radius
- **Hover:** Light gray background (#F3F4F6)
- **Selected:** Whisper Border background, Charcoal Ink text

### Tab Widget
- **Tab bar:** Transparent background
- **Tab:** Transparent background, Steel Secondary text, 2px transparent bottom border
- **Selected tab:** Charcoal Ink text, 2px Electric Blue bottom border
- **Hover tab:** Light gray background

### Scrollbar
- **Track:** Canvas Gray (#F5F5F5)
- **Thumb:** Whisper Border (#E5E7EB), 8px radius
- **Thumb hover:** Muted Slate (#9CA3AF)
- **Width:** 8px

---

## 5. Layout Principles

### Grid System
- **Sidebar:** 240px width (expandable/collapsible)
- **Chat area:** Flexible, fills remaining space
- **Input area:** Fixed height at bottom, 60-80px

### Spacing
- **Component gap:** 8px
- **Section gap:** 16px
- **Card internal padding:** 16px
- **Button padding:** 10px 24px

### Containment
- **Max content width:** None (full-width chat)
- **Sidebar max width:** 240px
- **Message max width:** 70% of chat area

---

## 6. Responsive Rules

This is a desktop application, so responsive rules are simplified:
- **Minimum window size:** 800x600
- **Sidebar collapse:** Below 900px width, sidebar collapses to 0px
- **Message bubbles:** Max width scales with window size
- **Font sizes:** Fixed, do not scale with window

---

## 7. Motion & Interaction

### Physics
- **Duration:** 200-250ms for standard transitions
- **Easing:** Ease-in-out for most transitions, ease-out for entrances
- **Sidebar collapse:** QPropertyAnimation with InOutCubic easing

### Micro-interactions
- **Button hover:** Background color transition (150ms)
- **Input focus:** Border color transition (150ms)
- **Message appear:** Fade-in opacity animation (250ms)
- **Sidebar toggle:** Width animation (200ms)

### Reduced Motion
- Respect system reduced motion settings
- Disable all animations when reduced motion is preferred

---

## 8. Language Style

### Tone
- **Professional:** Clear, concise, functional
- **Friendly:** Approachable, not overly formal
- **Efficient:** Minimal text, maximum clarity

### Labels
- **Buttons:** Verb-noun format ("Send Message", "Login", "Register")
- **Headers:** Noun format ("Chat", "Settings", "Users")
- **Errors:** Problem-solution format ("Password incorrect", "Username taken")

### Chinese Localization
- All UI text in Chinese for primary audience
- English fallback for technical terms
- Consistent terminology across all strings

---

## 9. Anti-Patterns (Banned)

### Visual
- No pure black (#000000) — always use Charcoal Ink (#1F2937)
- No pure white backgrounds — always use Canvas Gray (#F5F5F5)
- No neon outer glows or saturated gradients
- No dark themes — this is a light-theme-only system
- No custom mouse cursors
- No emoji in UI elements (except user messages)

### Typography
- No font sizes below 11px
- No font sizes above 20px for standard UI
- No mixed font families in the same element

### Layout
- No overlapping elements
- No horizontal scrollbars
- No elements touching window edges (minimum 8px padding)

### Interaction
- No animations longer than 300ms
- No infinite loop animations (except loading spinners)
- No sound effects on UI interactions

---

## Design Token Reference (QSS)

```
/* Primary Colors */
--canvas-gray: #F5F5F5;
--pure-surface: #FFFFFF;
--charcoal-ink: #1F2937;
--steel-secondary: #6B7280;
--muted-slate: #9CA3AF;
--whisper-border: #E5E7EB;

/* Accent Colors */
--electric-blue: #3B82F6;
--blue-hover: #2563EB;
--blue-pressed: #1D4ED8;

/* Semantic Colors */
--success-green: #10B981;
--warning-amber: #F59E0B;
--danger-red: #EF4444;

/* Typography */
--font-primary: "LXGW WenKai", "Microsoft YaHei UI", "Segoe UI", sans-serif;
--font-mono: "Cascadia Code", "Consolas", monospace;

/* Spacing */
--radius-sm: 4px;
--radius-md: 8px;
--radius-lg: 12px;
--padding-sm: 8px;
--padding-md: 16px;
--padding-lg: 24px;

/* Shadows */
--shadow-sm: 0 1px 2px rgba(0,0,0,0.05);
--shadow-md: 0 2px 8px rgba(0,0,0,0.08);
--shadow-lg: 0 4px 16px rgba(0,0,0,0.12);
```

---

*Last updated: 2026-06-19*
*Design system version: 1.0*
*Platform: Qt6 Widgets (Windows Desktop)*
