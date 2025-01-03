# zmk-layer-listeners

ZMK module to invoke certain behaviors on layer enter/leave.

## Usage

Add the following entries to `remotes` and `projects` in `config/west.yml`.

```yaml
manifest:
  remotes:
    - name: zmkfirmware
      url-base: https://github.com/zmkfirmware
    - name: ssbb
      url-base: https://github.com/ssbb
  projects:
    - name: zmk
      remote: zmkfirmware
      import: app/west.yml
    - name: zmk-layer-listeners
      remote: ssbb
  self:
    path: config
```

## Configuration

Layer listeners are specified like this:

```dts
/ {
    layer_listeners {
        compatible = "zmk,behavior-layer-listeners";

        // Call &haptic_feedback_in on layer enter, and &haptic_feedback_out on layer leave
        nav_num_feedback {
            layers = <NAV NUM>;
            bindings = <&haptic_feedback_in &haptic_feedback_out>;
        };

        // Call &reset_nav on NAV layer leave
        nav_reset {
            layers = <NAV>;
            bindings = <&none &reset_nav>;
        };
    };
}
```

### Behavior properties

- `tap-ms`: The time to wait (in milliseconds) between the press and release events of a triggered behavior. Defaults to 5 ms.
- `wait-ms`: The time to wait (in milliseconds) before triggering the next listener. Defaults to 5 ms.

### Listener Properties

Each listener is defined as a child node of this behavior.

- `layers` (required): A list of layers to which this listener should apply.
- `bindings` (required): The first behavior is triggered on layer entry, and the second on layer exit. Use `&none` for the other if you need only one.

## References

- [elpekenin/zmk-userspace](https://github.com/elpekenin/zmk-userspace) - Same thing with a different implementation and API.
- [badjeff/zmk-output-behavior-listener](https://github.com/badjeff/zmk-output-behavior-listener) - A more generic and complex listener that supports layers, keycodes, mouse events, etc.
