# 🟠 Amber Project UI Guidelines

## Visual Identity & Inspiration

The Amber interface finds its visual identity inside classic BMW dashboards from the 1990s (most notably E30, E34, E36, and E38 generations). The UI must exist in absolute harmony with the physical instruments surrounding it. 

---

## 1. Typography

* **Sizing & Scale:** Fixed, clear heights matching traditional console segment displays.
* **Layout:** Strictly flat, rectangular or uniform circular scaling. Avoid dynamic shifting or sliding layout schemes that make elements wobble.
* **Period-Correct Style:** Fonts should emulate either late-90s digital segment indicators or the cleanly cut, high-legibility sans-serif lettering printed on analog speedometers.

---

## 2. Color Palette

The interface is constrained to three primary colors to maintain dashboard integration.

* **Primary Amber:** `HEX: #FF7700` (Classic BMW 605nm orange-amber spectrum). High-intensity for active alerts, hands, and digital clock digits.
* **Deep Black:** `HEX: #000000` (Background state). Maximizes black level depth to fade the physical screen boundaries into the instrument cluster paneling.
* **Soft White / Gray:** `HEX: #D1D1D1` or `HEX: #FFFFFF`. Used sparingly for secondary status icons, gauge dial increments, and static markers.

---

## 3. Minimalism & OEM Look

* **No Flashy Displays:** Refuse anti-aliased gradients, drop shadows, or glassomorphic translucent windows.
* **Subtle Transitions:** Screen switches must occur in instant steps or simple, elegant frame swaps.
* **No Fluid/Elastic Effects:** Avoid modern bounce-back, elastic scrolling, or complex slide-out menus.

---

## 4. Animation Philosophy

* **Analog Replication:** Needle sweep on dials must behave with realistic physical movement (inertial dampening and gentle return).
* **Frame-Rates:** Kept stable to prevent flickering or stuttering.
* **Purpose over Flare:** Animation must only happen in response to user events or active state updates (such as shifting gears or turn-by-turn prompts).

---

## 5. Icons

* High contrast custom line-art style.
* Use geometry from standard warning lights found inside BMW clusters (low oil, ASC hazard triangle, battery, flat check-control symbols).
* No multi-colored emoji or detailed modern vectors.
