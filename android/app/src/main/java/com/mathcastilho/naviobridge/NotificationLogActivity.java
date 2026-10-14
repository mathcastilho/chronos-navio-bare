package com.mathcastilho.naviobridge;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.ActivityNotFoundException;
import android.content.ClipData;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import java.util.Locale;

public final class NotificationLogActivity extends Activity {
    private static final int REQUEST_EXPORT = 1;

    private Switch loggingSwitch;
    private TextView status;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        buildScreen();
    }

    private void buildScreen() {
        LinearLayout content = new LinearLayout(this);
        content.setOrientation(LinearLayout.VERTICAL);
        int padding = dp(20);
        content.setPadding(padding, padding, padding, padding);

        TextView title = new TextView(this);
        title.setText("Navigation notification log");
        title.setTextSize(24);
        title.setTextColor(0xFF17202A);
        content.addView(title);

        TextView description = new TextView(this);
        description.setText(
                "When enabled, every ongoing navigation notification from Google Maps, "
                        + "Waze, or OsmAnd is recorded before Navio Bridge parses it. The log "
                        + "includes all notification extras Android exposes, action/input "
                        + "metadata, and bitmap payloads. The log stays on this phone until you "
                        + "export or clear it and may contain sensitive destination details.");
        description.setTextSize(14);
        description.setPadding(0, dp(8), 0, dp(12));
        content.addView(description);

        loggingSwitch = new Switch(this);
        loggingSwitch.setText("Log raw navigation notifications");
        loggingSwitch.setChecked(NotificationLog.isEnabled(this));
        loggingSwitch.setPadding(0, dp(10), 0, dp(10));
        content.addView(loggingSwitch);
        loggingSwitch.setOnCheckedChangeListener((button, enabled) -> {
            NotificationLog.setEnabled(this, enabled);
            if (enabled) {
                sendBroadcast(new Intent(NavigationNotificationListener.ACTION_RELAY_CURRENT)
                        .setPackage(getPackageName()));
            }
            refreshStatus();
        });

        Button exportButton = new Button(this);
        exportButton.setText("Export log");
        exportButton.setOnClickListener(view -> createExportDocument());
        content.addView(exportButton);

        Button clearButton = new Button(this);
        clearButton.setText("Clear log");
        clearButton.setOnClickListener(view -> new AlertDialog.Builder(this)
                .setTitle("Clear notification log?")
                .setMessage("This permanently deletes the locally stored notification log.")
                .setNegativeButton("Cancel", null)
                .setPositiveButton("Clear", (dialog, which) ->
                        NotificationLog.clear(this,
                                () -> runOnUiThread(this::refreshStatus)))
                .show());
        content.addView(clearButton);

        status = new TextView(this);
        status.setTextSize(14);
        status.setTextColor(0xFF46515C);
        status.setPadding(0, dp(12), 0, 0);
        content.addView(status);

        ScrollView scrollView = new ScrollView(this);
        scrollView.setFillViewport(true);
        scrollView.addView(content);
        setContentView(scrollView);
        refreshStatus();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == REQUEST_EXPORT && resultCode == RESULT_OK && data != null) {
            Uri destination = data.getData();
            if (destination != null) {
                NotificationLog.export(this, destination,
                        error -> runOnUiThread(() -> showExportResult(error)));
            }
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        refreshStatus();
    }

    private void refreshStatus() {
        if (status == null) {
            return;
        }
        long bytes = NotificationLog.getSize(this);
        String error = getSharedPreferences(BleNavigationService.PREFERENCES, MODE_PRIVATE)
                .getString("notification_log_error", "");
        String message = String.format(
                Locale.ROOT, "Logging: %s\nCurrent log size: %.1f KB",
                NotificationLog.isEnabled(this) ? "on" : "off",
                bytes / 1024.0);
        if (!error.isEmpty()) {
            message += "\nLast log error: " + error;
        }
        status.setText(message);
    }

    private void showExportResult(String error) {
        refreshStatus();
        if (error.isEmpty()) {
            Toast.makeText(this, "Notification log exported.", Toast.LENGTH_LONG).show();
        } else {
            Toast.makeText(this, "Export failed: " + error, Toast.LENGTH_LONG).show();
        }
    }

    private void createExportDocument() {
        Intent intent = new Intent(Intent.ACTION_CREATE_DOCUMENT)
                .addCategory(Intent.CATEGORY_OPENABLE)
                .setType("text/plain")
                .putExtra(Intent.EXTRA_TITLE, "navio-notifications.jsonl");
        try {
            startActivityForResult(intent, REQUEST_EXPORT);
        } catch (ActivityNotFoundException exception) {
            shareLog();
        }
    }

    private void shareLog() {
        if (NotificationLog.getSize(this) == 0) {
            showExportResult("No notifications have been logged yet.");
            return;
        }
        Uri logUri = Uri.parse(
                "content://" + getPackageName()
                        + ".notification-log/navigation-notifications.jsonl");
        Intent sendIntent = new Intent(Intent.ACTION_SEND)
                .setType("text/plain")
                .putExtra(Intent.EXTRA_STREAM, logUri)
                .addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
        sendIntent.setClipData(ClipData.newRawUri("Navigation notification log", logUri));
        try {
            startActivity(Intent.createChooser(sendIntent, "Save or share notification log"));
        } catch (ActivityNotFoundException exception) {
            showExportResult("No installed app can save or share the log.");
        }
    }

    private int dp(int value) {
        return (int) (value * getResources().getDisplayMetrics().density + 0.5f);
    }
}
