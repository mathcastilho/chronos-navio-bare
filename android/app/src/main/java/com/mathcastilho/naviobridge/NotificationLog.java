package com.mathcastilho.naviobridge;

import android.content.Context;
import android.content.SharedPreferences;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.drawable.Drawable;
import android.graphics.drawable.Icon;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.service.notification.StatusBarNotification;
import android.util.Base64;
import android.util.Log;
import android.widget.RemoteViews;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.lang.reflect.Array;
import java.nio.charset.StandardCharsets;
import java.time.Instant;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.function.Consumer;

final class NotificationLog {
    static final String PREF_ENABLED = "notification_log_enabled";
    private static final String TAG = "NavioNotificationLog";
    private static final String FILE_NAME = "navigation-notifications.jsonl";
    private static final long MAX_LOG_BYTES = 10L * 1024L * 1024L;
    private static final ExecutorService IO = Executors.newSingleThreadExecutor();

    private NotificationLog() {
    }

    static boolean isEnabled(Context context) {
        return preferences(context).getBoolean(PREF_ENABLED, false);
    }

    static void setEnabled(Context context, boolean enabled) {
        preferences(context).edit().putBoolean(PREF_ENABLED, enabled).apply();
    }

    static void record(
            Context context,
            StatusBarNotification statusBarNotification,
            String event,
            NavigationText navigationText) {
        if (!isEnabled(context)) {
            return;
        }
        Context appContext = context.getApplicationContext();
        IO.execute(() -> {
            if (!isEnabled(appContext)) {
                return;
            }
            try {
                append(appContext, createRecord(
                        appContext, statusBarNotification, event, navigationText));
            } catch (JSONException | IOException | RuntimeException exception) {
                String message = exception.toString();
                preferences(appContext).edit().putString("notification_log_error", message).apply();
                Log.e(TAG, "Could not write notification log entry", exception);
            }
        });
    }

    static long getSize(Context context) {
        File file = logFile(context);
        return file.isFile() ? file.length() : 0;
    }

    static void clear(Context context, Runnable onComplete) {
        Context appContext = context.getApplicationContext();
        IO.execute(() -> {
            File file = logFile(appContext);
            boolean deleted = !file.exists() || file.delete();
            preferences(appContext).edit().remove("notification_log_error").apply();
            if (!deleted) {
                String message = "Could not delete the notification log.";
                preferences(appContext).edit().putString("notification_log_error", message).apply();
                Log.e(TAG, message);
            }
            onComplete.run();
        });
    }

    static void export(Context context, Uri destination, Consumer<String> onComplete) {
        Context appContext = context.getApplicationContext();
        IO.execute(() -> {
            String error = "";
            try {
                File file = logFile(appContext);
                if (!file.isFile()) {
                    throw new IOException("There are no logged notifications to export yet.");
                }
                try (OutputStream output = appContext.getContentResolver()
                        .openOutputStream(destination, "wt")) {
                    if (output == null) {
                        throw new IOException("Could not open the selected export destination.");
                    }
                    try (FileInputStream input = new FileInputStream(file)) {
                        byte[] buffer = new byte[8192];
                        int count;
                        while ((count = input.read(buffer)) != -1) {
                            output.write(buffer, 0, count);
                        }
                    }
                }
                preferences(appContext).edit().remove("notification_log_error").apply();
            } catch (IOException | RuntimeException exception) {
                error = exception.toString();
                preferences(appContext).edit().putString("notification_log_error", error).apply();
                Log.e(TAG, "Could not export notification log", exception);
            }
            try {
                onComplete.accept(error);
            } catch (RuntimeException exception) {
                Log.e(TAG, "Could not report notification log export result", exception);
            }
        });
    }

    private static JSONObject createRecord(
            Context context,
            StatusBarNotification statusBarNotification,
            String event,
            NavigationText navigationText)
            throws JSONException {
        android.app.Notification notification = statusBarNotification.getNotification();
        JSONObject record = new JSONObject();
        record.put("timestamp", Instant.now().toString());
        record.put("event", event);
        record.put("package", statusBarNotification.getPackageName());
        record.put("notificationKey", statusBarNotification.getKey());
        record.put("notificationId", statusBarNotification.getId());
        record.put("postTimeMillis", statusBarNotification.getPostTime());
        record.put("category", notification.category);
        record.put("flags", notification.flags);
        record.put("ongoing", (notification.flags
                & android.app.Notification.FLAG_ONGOING_EVENT) != 0);
        record.put("tickerText", jsonValue(context, notification.tickerText));
        record.put("whenMillis", notification.when);
        record.put("priority", notification.priority);
        record.put("visibility", notification.visibility);
        record.put("channelId", notification.getChannelId());
        record.put("group", notification.getGroup());
        record.put("sortKey", notification.getSortKey());
        record.put("extracted", extractedNavigation(navigationText));
        record.put("extras", jsonBundle(context, notification.extras));
        record.put("actions", jsonActions(context, notification.actions));
        record.put("largeIcon", jsonValue(context, notification.getLargeIcon()));
        record.put("contentView", jsonValue(context, notification.contentView));
        record.put("bigContentView", jsonValue(context, notification.bigContentView));
        record.put("headsUpContentView", jsonValue(context, notification.headsUpContentView));
        return record;
    }

    private static JSONObject extractedNavigation(NavigationText navigationText)
            throws JSONException {
        JSONObject extracted = new JSONObject();
        extracted.put("title", navigationText.title);
        extracted.put("duration", navigationText.duration);
        extracted.put("distance", navigationText.distance);
        extracted.put("eta", navigationText.eta);
        extracted.put("directions", navigationText.directions);
        extracted.put("speed", navigationText.speed);
        return extracted;
    }

    private static JSONArray jsonActions(
            Context context, android.app.Notification.Action[] actions) throws JSONException {
        JSONArray result = new JSONArray();
        if (actions == null) {
            return result;
        }
        for (android.app.Notification.Action action : actions) {
            JSONObject item = new JSONObject();
            item.put("title", jsonValue(context, action.title));
            item.put("icon", jsonValue(context, action.getIcon()));
            item.put("remoteInputs", jsonRemoteInputs(context, action.getRemoteInputs()));
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
                item.put("semanticAction", action.getSemanticAction());
            }
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
                item.put("contextual", action.isContextual());
            }
            item.put("allowGeneratedReplies", action.getAllowGeneratedReplies());
            result.put(item);
        }
        return result;
    }

    private static JSONArray jsonRemoteInputs(Context context, android.app.RemoteInput[] inputs)
            throws JSONException {
        JSONArray result = new JSONArray();
        if (inputs == null) {
            return result;
        }
        for (android.app.RemoteInput input : inputs) {
            JSONObject item = new JSONObject();
            item.put("resultKey", input.getResultKey());
            item.put("label", jsonValue(context, input.getLabel()));
            item.put("choices", jsonValue(context, input.getChoices()));
            item.put("allowFreeFormInput", input.getAllowFreeFormInput());
            item.put("allowedDataTypes", jsonValue(context, input.getAllowedDataTypes()));
            result.put(item);
        }
        return result;
    }

    private static JSONObject jsonBundle(Context context, Bundle bundle) throws JSONException {
        JSONObject result = new JSONObject();
        if (bundle == null) {
            return result;
        }
        bundle.setClassLoader(NotificationLog.class.getClassLoader());
        for (String key : bundle.keySet()) {
            try {
                result.put(key, jsonValue(context, bundle.get(key)));
            } catch (RuntimeException exception) {
                JSONObject error = new JSONObject();
                error.put("readError", exception.toString());
                result.put(key, error);
            }
        }
        return result;
    }

    private static Object jsonValue(Context context, Object value) throws JSONException {
        if (value == null) {
            return JSONObject.NULL;
        }
        if (value instanceof CharSequence) {
            return value.toString();
        }
        if (value instanceof Number || value instanceof Boolean) {
            return value;
        }
        if (value instanceof Bundle) {
            return jsonBundle(context, (Bundle) value);
        }
        if (value instanceof Bitmap) {
            Bitmap bitmap = (Bitmap) value;
            JSONObject image = new JSONObject();
            image.put("type", "Bitmap");
            image.put("width", bitmap.getWidth());
            image.put("height", bitmap.getHeight());
            image.put("pngBase64", encodeBitmap(bitmap));
            return image;
        }
        if (value instanceof Icon) {
            Icon icon = (Icon) value;
            JSONObject result = new JSONObject();
            result.put("type", "Icon");
            result.put("iconType", icon.getType());
            result.put("description", icon.toString());
            if (icon.getType() == Icon.TYPE_URI || icon.getType() == Icon.TYPE_URI_ADAPTIVE_BITMAP) {
                result.put("uri", icon.getUri());
            }
            try {
                Drawable drawable = icon.loadDrawable(context);
                if (drawable != null) {
                    int width = Math.max(1, drawable.getIntrinsicWidth());
                    int height = Math.max(1, drawable.getIntrinsicHeight());
                    Bitmap bitmap = Bitmap.createBitmap(
                            width, height, Bitmap.Config.ARGB_8888);
                    drawable.setBounds(0, 0, width, height);
                    drawable.draw(new Canvas(bitmap));
                    result.put("width", width);
                    result.put("height", height);
                    result.put("pngBase64", encodeBitmap(bitmap));
                    bitmap.recycle();
                }
            } catch (RuntimeException exception) {
                result.put("loadError", exception.toString());
            }
            return result;
        }
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P
                && value instanceof android.app.Person) {
            android.app.Person person = (android.app.Person) value;
            JSONObject result = new JSONObject();
            result.put("type", "Person");
            result.put("name", jsonValue(context, person.getName()));
            result.put("icon", jsonValue(context, person.getIcon()));
            result.put("uri", person.getUri());
            result.put("key", person.getKey());
            result.put("bot", person.isBot());
            result.put("important", person.isImportant());
            return result;
        }
        if (value instanceof RemoteViews) {
            RemoteViews views = (RemoteViews) value;
            JSONObject result = new JSONObject();
            result.put("type", "RemoteViews");
            result.put("package", views.getPackage());
            result.put("layoutId", views.getLayoutId());
            result.put("description", views.toString());
            return result;
        }
        if (value.getClass().isArray()) {
            JSONArray result = new JSONArray();
            int length = Array.getLength(value);
            for (int index = 0; index < length; index++) {
                result.put(jsonValue(context, Array.get(value, index)));
            }
            return result;
        }
        if (value instanceof Iterable<?>) {
            JSONArray result = new JSONArray();
            for (Object item : (Iterable<?>) value) {
                result.put(jsonValue(context, item));
            }
            return result;
        }
        JSONObject result = new JSONObject();
        result.put("type", value.getClass().getName());
        result.put("description", value.toString());
        return result;
    }

    private static String encodeBitmap(Bitmap bitmap) throws JSONException {
        ByteArrayOutputStream output = new ByteArrayOutputStream();
        if (!bitmap.compress(Bitmap.CompressFormat.PNG, 100, output)) {
            throw new JSONException("Could not encode notification bitmap.");
        }
        return Base64.encodeToString(output.toByteArray(), Base64.NO_WRAP);
    }

    private static void append(Context context, JSONObject record) throws IOException {
        File file = logFile(context);
        byte[] line = (record.toString() + "\n").getBytes(StandardCharsets.UTF_8);
        if (file.exists() && file.length() + line.length > MAX_LOG_BYTES) {
            try (FileOutputStream ignored = new FileOutputStream(file, false)) {
                // Start a fresh log rather than truncating an individual notification record.
            }
        }
        try (FileOutputStream output = new FileOutputStream(file, true)) {
            output.write(line);
            output.getFD().sync();
        }
    }

    private static File logFile(Context context) {
        return new File(context.getFilesDir(), FILE_NAME);
    }

    private static SharedPreferences preferences(Context context) {
        return context.getSharedPreferences(BleNavigationService.PREFERENCES, Context.MODE_PRIVATE);
    }
}
