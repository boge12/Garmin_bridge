# GarminBridge 3D-Printed Case

A simple snap-fit enclosure for the Seeed XIAO nRF52840.

## Overview

- **Design:** Two-part snap-fit box (base + lid)
- **External dimensions:** ~25 × 20 × 12 mm
- **Material:** PLA or PETG (recommended)
- **Print time:** ~20–30 minutes per part on a typical FDM printer

## Features

- USB-C port cutout on one end for power and firmware updates
- LED light pipe slot on the top face for the RGB status LED
- No screws, no glue — XIAO board press-fits into the base; lid snaps on
- Rounded corners to match the XIAO board's footprint

## Print Settings

| Setting | Value |
|---------|-------|
| Layer height | 0.2 mm |
| Infill | 15–20% (gyroid or grid) |
| Perimeters / walls | 3 |
| Material | PLA (easiest) or PETG (more durable) |
| Support | Not required |
| Bed adhesion | Brim recommended for the lid piece |

## Files

> **STL files coming soon.** The case is being designed in OpenSCAD for easy parametric customization.
> Check the [Releases page](https://github.com/boge12/Garmin_bridge/releases) for the latest STL download,
> or visit the project on [Printables.com](https://www.printables.com) (link to be added).

Once available, the archive will contain:
- `case_base.stl` — bottom half with XIAO mounting posts
- `case_lid.stl` — top half with LED slot
- `case.scad` — OpenSCAD source for customization

## Assembly

1. Print both parts.
2. Slide the XIAO nRF52840 into the base, USB-C port facing the cutout end.
3. Press the lid down until it snaps into place.
4. Plug in a USB-C cable to power the device.

## Customization

Once the OpenSCAD source is available, you can modify:
- Overall dimensions (if using a different board)
- Wall thickness
- Snap-fit tightness
- Add mounting holes or a cable clip

## Without a Case

The XIAO nRF52840 can be used without a case. It is small enough to tuck under a treadmill or tape to the frame. The board has no exposed high-voltage components, so bare-board use is safe as long as it's kept away from moisture.
