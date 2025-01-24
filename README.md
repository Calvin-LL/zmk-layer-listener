# zmk-layer-listener

ZMK module to invoke behaviors on layer change.

The bindings are triggered based on the highest layer.

Tested on the Glove80.

I use this to have per layer backlit (technically it's RGB underglow) colors.

## Usage

If you're using the Glove 80, first follow the instructions in https://github.com/moergo-sc/glove80-zmk-config-west.

Add the following entries to `remotes` and `projects` in `config/west.yml`.

```yaml
manifest:
  remotes:
    # ...
    - name: Calvin-LL
      url-base: https://github.com/Calvin-LL
  projects:
    # ...
    - name: zmk-layer-listener
      remote: Calvin-LL
      revision: main
  self:
    path: config
```

## Layer Listeners

You can also add this to the "Custom Device-tree" section of the [Glove80 Layout Editor](https://my.glove80.com) web app then export the keymap.

Layer listeners are specified like this:

```c
/ {
    layer_listeners {
        compatible = "zmk,layer-listeners";

        // white on base layer
        layer_base {
            layers = <LAYER_Base>;
            bindings = <&rgb_ug RGB_ON &rgb_ug RGB_COLOR_HSB(0,0,100)>;
        };

        // magenta on lower and magic layers
        layer_magic {
            layers = <LAYER_Lower LAYER_Magic>;
            bindings = <&rgb_ug RGB_ON &rgb_ug RGB_COLOR_HSB(300,100,100)>;
        };
    };
};
```

### Root properties

- `tap-ms`: The time to wait (in milliseconds) between the press and release events of a triggered behavior. Defaults to 5 ms.
- `wait-ms`: The time to wait (in milliseconds) before triggering the next listener. Defaults to 5 ms.

### Listener Properties

Each listener is defined as a child node.

- `layers` (required): A list of layers to which this listener should apply.
- `bindings` (required): All the behaviors to trigger when the layer is active.

## References

- [ssbb/zmk-listeners](https://github.com/ssbb/zmk-listeners/)
- [elpekenin/zmk-userspace](https://github.com/elpekenin/zmk-userspace) - Same thing with a different implementation and API.
- [badjeff/zmk-output-behavior-listener](https://github.com/badjeff/zmk-output-behavior-listener) - A more generic and complex listener that supports layers, keycodes, mouse events, etc.
