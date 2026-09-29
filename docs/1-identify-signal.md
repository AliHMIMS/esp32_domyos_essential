# 1. Identifying the bike's sensor signal

How we found out what the bike's 3.5 mm jack carries. This is already done for
this bike (see **Result**); the steps are kept for reference or for another bike.

## Result for this bike

| Question | Answer |
|---|---|
| Plug type | 2 contacts (TS: tip + sleeve) |
| Sensor type | Reed switch (magnet on the crank) |
| Pulses per pedal revolution | 1 |
| Reading when closed | about 1 Ω (beeps) |
| Reading when open | open circuit (`0.L` / `1`) |
| Behaviour when stopped | stays closed while the magnet is parked at the sensor (normal) |

The old display only counts these pulses; speed, distance, time and calories
are all calculated from them.

## What you need

- A multimeter (continuity mode with a beeper is ideal)
- Optional: 2 crocodile-clip leads so your hands stay free for the pedals

## Step 1: Look at the plug

Unplug the cable from the display and count the insulating rings on the plug:

```
  2 contacts (TS)            3 contacts (TRS)            4 contacts (TRRS)
  1 ring                     2 rings                     3 rings

   ___________                ___________                 ___________
  |           |==|>          |        |=|=|>             |      |=|=|=|>
  |  handle   |S |T          | handle |S|R|T             |handle|S|R2|R1|T
  |___________|==|>          |________|=|=|>             |______|=|=|=|>
                ^ ^                    ^ ^ ^                     ^ ^  ^  ^
            sleeve tip            sleeve ring tip             sleeve rings tip
```

- **T = Tip:** the pointed end
- **R = Ring:** the middle band(s)
- **S = Sleeve:** the long barrel next to the handle, usually GND

If you can see where the cable ends near the flywheel or crank, count the wires
at the sensor: 2 wires means a reed switch, 3 wires means a Hall sensor.

## Step 2: Set up the multimeter

Set the dial to **continuity** (the diode/speaker symbol). Without it, use
**resistance on the 200 Ω or 2 kΩ range**.

```
        ┌─────────────────────┐
        │      [  0.L  ]      │   <- "0.L" / "1" = open circuit (no contact)
        │                     │
        │   V~  V⎓   Ω  •)))  │
        │        \   |  /     │
        │    ──── (dial) ──── │   <- point it at •))) or Ω
        │                     │
        │  [10A] [COM] [VΩ]   │
        └─────────┬─────┬─────┘
                  │     │
               black   red
```

Touch the probes together: you should see about 0 Ω and hear a beep.

## Step 3: Connect to each pair of contacts

For a 3-contact plug, test all 3 pairs: Tip–Sleeve, Ring–Sleeve and Tip–Ring.
For a 2-contact plug there is only Tip–Sleeve.

```
                    red clip                black clip
                       │                        │
   ___________        ▼                        ▼
  |           |==|=|=|>  ← clip on the TIP
  |  handle   |S |R|T
  |___________|==|=|=|>
               ▲
               └── clip on the SLEEVE (the long barrel)
```

Wrap a strip of tape around the insulating ring so the clips can't touch two
sections at once.

## Step 4: Turn the pedals slowly

Turn the pedals by hand, about one turn every 3–4 seconds, and watch the meter.

```
Crank position:   0°      90°     180°     270°     360°(=0°)
Meter reading:  0.L ... 0.L ... [ 0.3Ω ] ... 0.L ... 0.L ... [ 0.3Ω ]
Beeper:                          BEEP!                        BEEP!
                                   ▲
                       the magnet is passing the sensor

 open   ────────┐   ┌──────────────────────┐   ┌──────────
                │   │                      │   │
 closed         └───┘                      └───┘
               1 pulse                    1 pulse
          <──────── one revolution ────────>
```

Count the beeps per full pedal revolution.

## Step 5: Interpret the result

```
                      Did one pair toggle 0Ω ⇄ open?
                               │
                ┌──────────────┴──────────────┐
               YES                            NO
                │                              │
     ✅ REED SWITCH                     Is it a 3-contact plug?
     Those 2 contacts go to the ESP32:         │
       one → GPIO (with pull-up)       ┌───────┴───────┐
       other → GND                    YES              NO
                                       │                │
                                Possibly a HALL     Magnet missing
                                sensor (needs       or cable broken?
                                power): Step 6.     Check them.
```

## Step 6: Only if nothing toggled — checking for a Hall sensor

A Hall sensor needs power, so measure with the display connected and switched on.
Put a female breakout and a male-to-male 3.5 mm cable between the sensor cable and
the display, so you can reach the contacts:

```
  bike sensor ──cable──> [ female breakout ] ──male-male cable──> display
                           │  │  │
                           T  R  S   ← probe these terminals
```

Set the meter to **DC volts, 20 V range**, with the black probe on the Sleeve:

| Contact | At rest | While turning slowly | Meaning |
|---|---|---|---|
| Ring | ~3.0 V, steady | steady | power (VCC) |
| Tip | ~3.0 V or ~0 V | flips 3 V ⇄ 0 V | signal |
| Sleeve | 0 V | – | GND |

If the voltages stay below 3.3 V, the ESP32 can read the signal directly.
