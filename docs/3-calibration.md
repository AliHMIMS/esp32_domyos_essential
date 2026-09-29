# 3. Calibrating against the old display

The old display calculates speed, distance and calories from the same pedal
pulses as the ESP32, using fixed constants of its own. Calibration finds those
constants so both show the same numbers. All values are entered in the web UI
under **Settings** and are saved on the board.

| Setting | Default | What it controls |
|---|---|---|
| Metres per revolution | 5.0 | speed and distance (60 rpm = 18 km/h) |
| Weight (kg) | 70 | calories |
| Power factor | 0.020 | estimated power at the reference knob level: W = factor × rpm² |
| Calorie multiplier | 1.0 | final scaling of calories |
| Pulses per revolution | 1 | magnet passes per pedal turn (measured: 1) |
| Debounce (ms) | 60 | ignores contact bounce shorter than this |
| Level multipliers | 0.55 … 2.22 | power at each knob level, relative to the reference level (section C) |

Do the steps **in order**: calories depend on speed, so fix speed and distance
first, then calories, then the knob levels.

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
its calories depend on speed and time only. The ESP32 also uses the knob level
you select, so the two can only match at one level: do this section at your
**reference level** (the level you usually ride, 4 by default), whose
multiplier is 1.0. Section C then scales the other levels from it.

Do speed and distance (section A) first, so both rides really are at the same
pace.

### Step 1: ride on the old display

1. Plug the bike cable into the **old display** and reset it to 0.
2. Set the tension knob to your reference level.
3. Set the metronome to your usual cadence, e.g. **70 bpm**, and pedal one turn
   per beat for exactly **10 minutes**.
4. Write down:

   ```
   Old display:   time 10:00   distance ____ km   calories ____ kcal
   ```

### Step 2: the same ride on the ESP32

1. Plug the bike cable into the **ESP32**.
2. In the web UI, tap **New ride**.
3. Same tension knob level, selected under **Knob level** on the dashboard too,
   and the same metronome cadence, for exactly **10 minutes**.
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
power (W)        = power factor × level multiplier × rpm²
oxygen use       = 7 + 10.8 × power ÷ weight        (ml per kg per minute)
kcal per minute  = oxygen use × weight ÷ 1000 × 5   (1 litre O₂ ≈ 5 kcal)
calories         = sum of kcal per minute × calorie multiplier, while moving
```

The oxygen formula is the ACSM equation for leg cycle ergometry. The resting
share (the `7`) is included, as on most fitness displays.

Neither the old display nor the ESP32 measures the real resistance, so both
are estimates. Matching them makes the numbers consistent with your history; it
doesn't make them exact.

---

## C. Knob levels (level multipliers)

The tension knob has 8 levels. The bike can't report which level is set, so you
choose it on the dashboard (**Knob level** 1–8) whenever you turn the knob.
Bluetooth apps receive it too, as the FTMS resistance level. Each level has a
power multiplier:

```
power (W) = power factor × level multiplier × rpm²
```

The reference level has multiplier 1.0 and uses the power factor from section B
unchanged. Until you calibrate, the multipliers are a guess (0.55 at level 1 up
to 2.22 at level 8, reference level 4).

The bike has a freewheel, so the ESP32 can't measure the brake directly. The
wizard uses **effort matching** instead: the same effort means the same power,
so if level 4 at 70 rpm feels as hard as level 6 at 57 rpm, level 6's
multiplier is (70 ÷ 57)² = 1.51.

### Running the wizard (about 18 minutes, hands-free)

1. Warm up for 5 minutes. Turn the phone's sound up and set its screen to
   stay on. A heart-rate watch helps a lot: keep the same heart rate the whole
   time. Without one, keep the same breathing.
2. Open **Settings → Knob levels → Calibrate levels**.
3. Pick the **reference level**: the level you usually ride, and the one you
   used for the calories in section B. It gets multiplier 1.0.
4. Turn the knob to **1**, start pedalling and tap **Start on level 1**. From
   here the page runs by itself:
   - **Level 1 (about 2.5 min):** settle into a steady, moderate effort that
     you could also hold on level 8 by pedalling slowly (about 80–90 rpm on
     level 1). That effort is the target for every level. The last 60 s are
     measured.
   - **Levels 2 to 8 (about 2 min each):** two beeps and a voice say "Turn to
     level N". Turn the knob within 10 s and keep the **same effort**, which
     means pedalling slower on each heavier level. After 45 s to settle, a
     short beep starts the 60 s measurement, and three ticks count down to the
     next level. Match the effort, not the cadence.
   - **Fatigue check:** back to level 1 at the same effort. This shows whether
     your effort drifted over the session.
5. A final beep and "Calibration done" mark the end. Check the results table
   and tap **Save**, or **Discard**.

If you stop pedalling during a measurement, the wizard says so and measures
that level again after 20 s. **Skip level** moves on when a level can't be
matched, and it's estimated from the levels on either side. **Pause** stops the
timer; **Resume** restarts the current level.

If the results warn that the fatigue check is off by more than 8 %, or that
a heavier level came out lighter than the one below it, rest and redo the
calibration. You can also edit any
multiplier by hand under **Settings → Knob levels**.

---

## Record of calibration values

Keep the values you end up with here, so they can be restored after a reset:

| Date | Metres per rev | Weight | Power factor | Calorie multiplier | Level multipliers 1–8 (ref) | Notes |
|---|---|---|---|---|---|---|
| 2026-09-29 | 5.0 | 70 | 0.020 | 1.0 | 0.55 0.67 0.82 1.00 1.22 1.49 1.82 2.22 (4) | defaults, not calibrated yet |
