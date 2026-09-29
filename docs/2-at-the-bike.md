# 2. Setting up at the bike

How to connect the ESP32 to the bike and start riding with the web UI and
Bluetooth apps. The firmware must already be flashed (see the README).

## What to bring

- The ESP32-C6 board
- A USB-C cable and a **USB charger** (any phone charger, 5 V, 1 A or more), or a
  power bank
- One way to connect the bike's plug to the board:
  - **Easiest:** a 3.5 mm female jack breakout with screw terminals, plus 2
    female-to-male jumper wires
  - **Or:** 2 crocodile-clip leads plus 2 female-to-male jumper wires
- Your phone

## Step 1: Unplug the bike cable from the old display

Pull the 3.5 mm plug out of the old display. You use either the old display or
the ESP32, not both at once.

## Step 2: Find the two pins on the board

The pin labels are printed on the board next to the pins. Find:

- **`2`**: GPIO2 (signal)
- **`G`** or **`GND`**: ground. There are several, and any one works.

Go by the printed label, not the position:

```
          ESP32-C6 board (top view, USB-C ports at the bottom)

      ┌───────────────────────────────┐
      │ 3V3                       G   │
      │ RST                      TX   │
      │  4                       RX   │
      │  5      ┌───────────┐    15   │
      │  6      │  ESP32-C6 │    23   │
      │  7      │  WROOM-1  │    22   │
      │  0      └───────────┘    21   │
      │  1                       20   │
      │  8                       19   │
      │ 10                       18   │
      │ 11                        9   │
      │  2  ◄── signal (tip)      G   │ ◄── ground (sleeve)
      │  3                       13   │
      │ 5V                       12   │
      │  G  ◄── or this ground    G   │
      │      [CH343]      [ESP32C6]   │
      └────────┴──────────────┴───────┘
               USB-C          USB-C
```

This layout is only an example; check the labels on your board.

## Step 3: Connect the plug to the board

**Option A: with a jack breakout**

```
 bike cable plug ──plug into──> [ 3.5mm female breakout ]
                                   T ●───── jumper ─────> pin "2"
                                   S ●───── jumper ─────> pin "G"
                                   (R ● if present: leave empty)
```

1. Plug the bike's plug into the breakout. It should click in fully.
2. Connect a jumper from the **T** terminal (tip, sometimes marked L) to **pin 2**.
3. Connect a jumper from the **S** terminal (sleeve, sometimes marked GND or G)
   to a **G** pin.

**Option B: crocodile clips on the plug**

```
             clip 1              clip 2
               │                   │
               ▼                   ▼
   ___________
  |           |==|>  ◄── clip 1 on the TIP (the pointed end)
  |  handle   |S |T
  |___________|==|>
               ▲
               └── clip 2 on the SLEEVE (the barrel next to the handle)

   clip 1 ── jumper ──> pin "2"
   clip 2 ── jumper ──> pin "G"
```

1. Wrap tape around the black ring between the tip and the sleeve, so the clips
   can't touch each other.
2. Put clip 1 on the **tip** and clip 2 on the **sleeve**.
3. Push the female end of one jumper onto **pin 2** and clip clip 1 onto the
   jumper's male end.
4. Do the same for clip 2 to a **G** pin.

It doesn't matter which way round the tip and sleeve go, because a reed switch
has no polarity. The only mistake to avoid is connecting either wire to **3V3**
or **5V**.

Optional, for a cleaner signal: a 10 kΩ resistor from pin 2 to 3V3 and a 100 nF
capacitor from pin 2 to G.

## Step 4: Power the board

1. Plug the USB-C cable into **either** USB-C port. Both work for power.
2. Plug the other end into the charger or power bank.
3. Wait about **15 seconds** for it to start and join the Wi-Fi.

Some power banks switch off after a while because the ESP32 draws very little
current. If the page goes offline after a few minutes, use a wall charger.

## Step 5: Open the web page on your phone

1. Make sure your phone is on the **home Wi-Fi** (not mobile data or another
   network).
2. In the browser, go to **`http://domyos.local`**.
3. If that doesn't load (common on older Android phones), use the board's IP
   address instead, e.g. **`http://192.168.1.9`**. The current IP is shown in
   Settings → Wi-Fi, and also in your router's list of devices.
4. You should see **Domyos Ride**, with a green **`live`** label in the top right.

Tip: add the page to your home screen so it opens like an app. On iPhone, tap
Share → "Add to Home Screen"; on Android, tap ⋮ → "Add to Home screen".

## Step 6: Check the pulses

1. Tap **Settings** at the bottom of the page.
2. Under **Calibration**, tap **Zero**. The counter shows 0.
3. Turn the pedals **exactly 20 full turns** at a normal pace.
4. Read the counter:

| Counter shows | Meaning | What to do |
|---|---|---|
| **20** | Everything is correct | Go to Step 7 |
| **More than 20** (e.g. 23, 40) | The switch is bouncing, giving extra pulses | Raise **Debounce (ms)** from 60 to 120, tap **Save**, test again |
| **Fewer than 20** | Debounce is dropping real pulses (unlikely) | Lower **Debounce (ms)** to 40, save, test again |
| **0** | No signal is arriving | See "The counter stays at 0" below |

5. Tap **Back to ride** and pedal. The label changes to **`riding`**, speed and
   cadence rise, and time starts counting. Stop pedalling and after 3 seconds the
   label changes to **`paused`**. Time only counts while you're moving.

## Step 7: Connect a training app over Bluetooth (optional)

1. Open your training app, for example Kinomap, Zwift, MyWhoosh or Rouvy.
2. Open its **device** or **sensor** settings (often under "Pair" or
   "Equipment"). Choose **Smart trainer** or **Fitness machine** (FTMS).
3. Select **Domyos Essential**.

- **Don't pair it in the phone's Bluetooth settings.** Connect from inside the
  app, otherwise the app won't find it.
- Only **one app** can connect over Bluetooth at a time. The web page keeps
  working alongside it, and its **`BLE`** label turns green (**`BLE app`**)
  while an app is connected.
- The bike has a manual tension knob, so apps can't change resistance. They get
  speed, cadence, distance, estimated power, calories and time.

## Troubleshooting

**The page doesn't load at all**

- Check that your phone is on the home Wi-Fi and that mobile data isn't taking
  over.
- Try the IP address, typing `http://` explicitly.
- If the home Wi-Fi can't be reached, the board opens its own hotspot after 15
  seconds. Connect your phone to **Domyos-Setup** (password `domyos123`) and open
  **`http://192.168.4.1`**. If this happens at the bike, the board is too far
  from the router.

**The page loads but says `offline`**

The board has lost Wi-Fi or power. Check the power bank, or move the board
closer to the router.

**The counter stays at 0**

1. Make sure the wires are on pin **2** and a **G** pin, not a neighbouring pin.
2. Make sure the jumpers are pushed fully onto the pins and the clips aren't
   touching each other.
3. Make sure the plug is pushed fully into the breakout.
4. Quick test: disconnect the wire from the tip, then briefly touch the
   **pin 2** jumper to the **G** jumper a few times. Each touch should add 1 to
   the counter.
   - **It counts:** the board works, and the problem is between the plug and the
     breakout or clips.
   - **It doesn't count:** the board or pin is at fault.

## Next

Once the pulse count is right, calibrate speed, distance and calories against
the old display: see [3-calibration.md](3-calibration.md).
