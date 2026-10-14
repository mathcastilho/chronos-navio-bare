package com.mathcastilho.naviobridge;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.Switch;
import android.widget.TextView;

public final class DeviceSettingsActivity extends Activity {
    private static final int[] TIMEOUT_VALUES = {0, 1, 2, 3, 4};
    private static final String[] TIMEOUT_LABELS = {
            "5 seconds", "10 seconds", "20 seconds", "30 seconds", "Always on"
    };
    private static final String[] THEME_LABELS = {
            "White", "Blue", "Green", "Amber"
    };
    private static final int[] THEME_VALUES = {
            0xFFFFFF, 0x2196F3, 0x4CAF50, 0xFFC107
    };

    private DeviceSettings settings;
    private LinearLayout content;
    private TextView brightnessValue;
    private TextView iconSizeValue;
    private SeekBar brightnessSeekBar;
    private SeekBar iconSizeSeekBar;
    private Spinner timeoutSpinner;
    private Spinner themeSpinner;
    private Spinner headingPositionSpinner;
    private Switch etaSwitch;
    private Switch directionsSwitch;
    private Switch largeDirectionsSwitch;
    private Switch tripInfoSwitch;
    private TextView saveStatus;
    private boolean ready;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        settings = DeviceSettings.load(this);
        buildScreen();
    }

    private void buildScreen() {
        content = new LinearLayout(this);
        content.setOrientation(LinearLayout.VERTICAL);
        int padding = dp(20);
        content.setPadding(padding, padding, padding, padding);

        TextView title = new TextView(this);
        title.setText("Chronos Navio settings");
        title.setTextSize(24);
        title.setTextColor(0xFF17202A);
        content.addView(title);

        TextView description = new TextView(this);
        description.setText(
                "These values are saved on your phone and sent to the ESP32 when connected.");
        description.setTextSize(14);
        description.setPadding(0, dp(8), 0, dp(12));
        content.addView(description);

        brightnessValue = addLabel("Brightness: " + settings.brightness + "%");
        brightnessSeekBar = new SeekBar(this);
        brightnessSeekBar.setMax(100);
        brightnessSeekBar.setProgress(settings.brightness);
        content.addView(brightnessSeekBar);
        brightnessSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                brightnessValue.setText("Brightness: " + progress + "%");
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                settings.brightness = seekBar.getProgress();
                saveAndSend();
            }
        });

        timeoutSpinner = addSpinner(
                "Screen timeout", TIMEOUT_LABELS, settings.timeout);

        etaSwitch = addSwitch("Show estimated arrival time", settings.showEta);
        directionsSwitch = addSwitch("Show maneuver directions", settings.showDirections);
        largeDirectionsSwitch = addSwitch("Use large directions text", settings.largeDirections);
        tripInfoSwitch = addSwitch("Show trip information", settings.showTripInfo);
        headingPositionSpinner = addSpinner(
                "Maneuver heading position",
                new String[]{"Below icon", "Above icon"},
                settings.headingAboveIcon ? 1 : 0);

        iconSizeValue = addLabel("Navigation icon size: " + settings.iconSize);
        iconSizeSeekBar = new SeekBar(this);
        iconSizeSeekBar.setMax(512 - 256);
        iconSizeSeekBar.setProgress(settings.iconSize - 256);
        content.addView(iconSizeSeekBar);
        iconSizeSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                iconSizeValue.setText("Navigation icon size: " + (progress + 256));
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                settings.iconSize = seekBar.getProgress() + 256;
                saveAndSend();
            }
        });

        themeSpinner = addSpinner("Theme color", THEME_LABELS, themeIndex(settings.themeColor));

        saveStatus = addLabel("");
        addSpinnerListener(timeoutSpinner,
                position -> settings.timeout = TIMEOUT_VALUES[position]);
        addSpinnerListener(themeSpinner,
                position -> settings.themeColor = THEME_VALUES[position]);
        addSpinnerListener(headingPositionSpinner,
                position -> settings.headingAboveIcon = position == 1);
        addSwitchListener(etaSwitch, value -> settings.showEta = value);
        addSwitchListener(directionsSwitch, value -> settings.showDirections = value);
        addSwitchListener(largeDirectionsSwitch, value -> settings.largeDirections = value);
        addSwitchListener(tripInfoSwitch, value -> settings.showTripInfo = value);
        ready = true;

        ScrollView scrollView = new ScrollView(this);
        scrollView.setFillViewport(true);
        scrollView.addView(content);
        setContentView(scrollView);
    }

    private Spinner addSpinner(String label, String[] options, int selection) {
        addLabel(label);
        Spinner spinner = new Spinner(this);
        spinner.setAdapter(new ArrayAdapter<>(
                this, android.R.layout.simple_spinner_dropdown_item, options));
        spinner.setSelection(Math.max(0, Math.min(selection, options.length - 1)));
        content.addView(spinner);
        return spinner;
    }

    private Switch addSwitch(String label, boolean checked) {
        Switch control = new Switch(this);
        control.setText(label);
        control.setTextSize(16);
        control.setChecked(checked);
        control.setPadding(0, dp(10), 0, dp(10));
        content.addView(control);
        return control;
    }

    private TextView addLabel(String text) {
        TextView label = new TextView(this);
        label.setText(text);
        label.setTextSize(15);
        label.setTextColor(0xFF46515C);
        label.setPadding(0, dp(12), 0, dp(2));
        content.addView(label);
        return label;
    }

    private void addSpinnerListener(Spinner spinner, IntAction action) {
        int initialSelection = spinner.getSelectedItemPosition();
        boolean[] receivedInitialSelection = {false};
        spinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(
                    AdapterView<?> parent, View view, int position, long id) {
                action.run(position);
                boolean isInitialSelection = !receivedInitialSelection[0];
                receivedInitialSelection[0] = true;
                if (ready && (!isInitialSelection || position != initialSelection)) {
                    saveAndSend();
                }
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {
            }
        });
    }

    private void addSwitchListener(Switch control, BooleanAction action) {
        control.setOnCheckedChangeListener((buttonView, checked) -> {
            action.run(checked);
            if (ready) {
                saveAndSend();
            }
        });
    }

    private int themeIndex(int color) {
        for (int index = 0; index < THEME_VALUES.length; index++) {
            if (THEME_VALUES[index] == color) {
                return index;
            }
        }
        return 0;
    }

    private void saveAndSend() {
        settings.save(this);
        if (getSharedPreferences(BleNavigationService.PREFERENCES, MODE_PRIVATE)
                .getBoolean(BleNavigationService.PREF_ENABLED, false)) {
            startService(new android.content.Intent(this, BleNavigationService.class)
                    .setAction(DeviceSettings.ACTION_UPDATE)
                    .putExtra(DeviceSettings.EXTRA_PACKET, settings.toPacket()));
            saveStatus.setText("Saved and sent to Chronos Navio.");
        } else {
            saveStatus.setText("Saved. It will be sent when Navio Bridge connects.");
        }
    }

    private int dp(int value) {
        return (int) (value * getResources().getDisplayMetrics().density + 0.5f);
    }

    private interface IntAction {
        void run(int value);
    }

    private interface BooleanAction {
        void run(boolean value);
    }
}
