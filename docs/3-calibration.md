# 3. Calibrating against the old display

The old display calculates speed, distance and calories from the same pedal
pulses as the ESP32, using fixed constants of its own. Calibration finds those
constants so both show the same numbers. All values are entered in the web UI
under **Settings → Bike & rider** and are saved on the board.

| Setting | Default | What it controls |
|---|---|---|
| Metres per revolution | 5.0 | speed and distance (60 rpm = 18 km/h) |
| Weight (kg) | 70 | calories |
| Power factor | 0.020 | estimated power: W = factor × rpm² |
| Calorie multiplier | 1.0 | final scaling of calories |
| Pulses per revolution | 1 | magnet passes per pedal turn (measured: 1) |
| Debounce (ms) | 60 | ignores contact bounce shorter than this |

Do the steps **in order**: calories depend on speed, so fix speed and distance
first.

You need the old display, a way to swap the bike cable between the old display
and the ESP32, and ideally a **metronome app** on your phone to hold a steady
cadence.

---

## A. Speed and distance (metres per revolution)

Pick **one** of these methods. Method 1 is quickest; method 2 is more accurate.

### Method 1: speed at a fixed cadence (5 minutes)

1. Plug the bike cable into the **old display**.
2. Set a metronome to **60 bpm** and pedal **one full turn per beat** (60 rpm).
3. Once it's steady, read the **speed** on the old display, e.g. `18.5 km/h`.
4. Calculate:

   ```
   metres per revolution = speed (km/h) × 1000 ÷ (60 × rpm)

   example: 18.5 × 1000 ÷ (60 × 60) = 5.14
   ```

5. Enter the result under **Metres per revolution** and tap **Save**.

### Method 2: distance over a counted ride (10 minutes)

1. Plug the bike cable into the **old display** and reset it to 0.
2. Set a metronome to **60 bpm** and pedal one turn per beat for exactly
   **10 minutes** (= 600 turns). Or count the turns yourself.
3. Read the **distance** on the old display, e.g. `3.08 km`.
4. Calculate:

   ```
   metres per revolution = distance (km) × 1000 ÷ turns

   example: 3.08 × 1000 ÷ 600 = 5.13
   ```

5. Enter the result under **Metres per revolution** and tap **Save**.

### Check

Plug the cable into the ESP32 and pedal at the same 60 bpm. The web UI should
show the same speed the old display did, within about 0.3 km/h.

---

## B. Calories (calorie multiplier)

The old display can't know the tension knob setting (it only sees pulses), so
its calories depend on speed and time only. The ESP32's estimate works the same
way, from cadence and your weight. So a single multiplier is enough to match the
old display.

Do speed and distance (section A) first, so both rides really are at the same
pace.

### Step 1: ride on the old display

1. Plug the bike cable into the **old display** and reset it to 0.
2. Set the tension knob to your usual setting and **note it**.
3. Set the metronome to your usual cadence, e.g. **70 bpm**, and pedal one turn
   per beat for exactly **10 minutes**.
4. Write down:

   ```
   Old display:   time 10:00   distance ____ km   calories ____ kcal
   ```

### Step 2: the same ride on the ESP32

1. Plug the bike cable into the **ESP32**.
2. In the web UI, tap **New ride**.
3. Same tension knob setting, same metronome cadence, for exactly **10 minutes**.
   The web UI's **Time** must show 10:00 (it pauses when you stop pedalling).
4. Check that the **distance** is close to the old display's. If it isn't, redo
   section A first.
5. Write down:

   ```
   ESP32:         time 10:00   distance ____ km   calories ____ kcal
   ```

### Step 3: calculate and save the multiplier

1. Open **Settings** and note the current **Calorie multiplier** (1.0 the first
   time).
2. Calculate:

   ```
   new multiplier = current multiplier × old display kcal ÷ ESP32 kcal

   example: old display 62 kcal, ESP32 84 kcal, current 1.0
            1.0 × 62 ÷ 84 = 0.74
   ```

3. Enter the new value under **Calorie multiplier** and tap **Save**. It applies
   from now on; the current ride's total isn't recalculated.

### Step 4 (optional): check at a different pace

1. Repeat Step 1 and Step 2 at a clearly different cadence, e.g. **90 bpm**.
2. Compare the two results:
   - **Both kcal values now match** (within about 5 %): you're done.
   - **The ESP32 is too high at the fast pace and right at the slow one:** lower
     the **Power factor** (e.g. 0.020 → 0.015), then redo Step 3 at your usual
     pace.
   - **The ESP32 is too low at the fast pace:** raise the **Power factor**
     (e.g. 0.020 → 0.025), then redo Step 3.

The power factor controls how quickly calories rise with cadence; the multiplier
scales everything evenly.

### How the estimate works

For reference, the firmware calculates (in `src/bike.cpp`):

```
power (W)        = power factor × rpm²
oxygen use       = 7 + 10.8 × power ÷ weight        (ml per kg per minute)
kcal per minute  = oxygen use × weight ÷ 1000 × 5   (1 litre O₂ ≈ 5 kcal)
calories         = sum of kcal per minute × calorie multiplier, while moving
```

The oxygen formula is the ACSM equation for leg cycle ergometry. The resting
share (the `7`) is included, as on most fitness displays.

Neither the old display nor the ESP32 knows the real resistance, so both are
estimates. Matching them makes the numbers consistent with your history; it
doesn't make them exact.

---

## Record of calibration values

Keep the values you end up with here, so they can be restored after a reset:

| Date | Metres per rev | Weight | Power factor | Calorie multiplier | Notes |
|---|---|---|---|---|---|
| 2026-09-29 | 5.0 | 70 | 0.020 | 1.0 | defaults, not calibrated yet |
