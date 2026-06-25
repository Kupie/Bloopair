# Bloopair SD Card Configuration

Bloopair loads JSON configuration files from `sd:/wiiu/bloopair/`.

## Wiimote emulation mode

Wiimote emulation is enabled per controller BDA by placing a configuration file whose filename is `Controller-` plus the controller Bluetooth address in uppercase hexadecimal. For example, a controller with BDA `01:23:45:67:89:AB` uses:

```text
sd:/wiiu/bloopair/Controller-0123456789AB.conf
```

The file must use the existing Bloopair configuration shape: `controllerType`, optional `configuration`, optional `mapping`, and optional `custom`.

```json
{
  "controllerType": "Wiimote",
  "custom": {
    "extensionMode": "none",
    "irToggleButton": "stick_r",
    "irVelocity": 50
  },
  "mapping": {
    "up": [20],
    "left": [18],
    "down": [21],
    "right": [19],
    "2": [4],
    "1": [5],
    "a": [6],
    "b": [7],
    "minus": [15, 14],
    "plus": [1, 0],
    "home": [9]
  }
}
```

`mapping` uses the existing loader format: each object key is the Bloopair destination button name, and each numeric array lists source button IDs from the physical controller module. The example above matches the built-in Switch Pro to sideways Wiimote defaults.

Wiimote custom options:

* `extensionMode`: `"none"` for a bare Wiimote, or `"nunchuk"` for the reserved Nunchuk extension ID stub. Nunchuk input reports are not implemented yet.
* `irToggleButton`: Bloopair button name used to toggle the fake IR pointer. Defaults to `"stick_r"`.
* `irVelocity`: linear pointer velocity scale from `1` to `100`. Defaults to `50`.
