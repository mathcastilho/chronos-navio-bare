package com.mathcastilho.naviobridge;

import android.app.Notification;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.drawable.Drawable;
import android.os.Build;
import android.os.Parcelable;
import android.graphics.drawable.Icon;

final class NavigationIcon {
    static final int SIZE = 48;
    static final int ROW_BYTES = SIZE / 8;
    static final int BYTE_COUNT = SIZE * ROW_BYTES;

    private NavigationIcon() {
    }

    static byte[] fromNotification(Context context, Notification notification) {
        Bitmap bitmap = getLargeIconBitmap(notification);
        if (bitmap != null) {
            return fromBitmap(bitmap);
        }

        Icon icon = notification.getLargeIcon();
        if (icon == null) {
            return null;
        }
        Drawable drawable = icon.loadDrawable(context);
        if (drawable == null) {
            return null;
        }
        Bitmap rendered = Bitmap.createBitmap(SIZE, SIZE, Bitmap.Config.ARGB_8888);
        Canvas canvas = new Canvas(rendered);
        int width = drawable.getIntrinsicWidth();
        int height = drawable.getIntrinsicHeight();
        if (width <= 0 || height <= 0) {
            width = SIZE;
            height = SIZE;
        }
        float scale = Math.min((float) SIZE / width, (float) SIZE / height);
        int scaledWidth = Math.round(width * scale);
        int scaledHeight = Math.round(height * scale);
        int left = (SIZE - scaledWidth) / 2;
        int top = (SIZE - scaledHeight) / 2;
        drawable.setBounds(left, top, left + scaledWidth, top + scaledHeight);
        drawable.draw(canvas);
        return fromBitmap(rendered);
    }

    static byte[] fromBitmap(Bitmap bitmap) {
        int width = bitmap.getWidth();
        int height = bitmap.getHeight();
        int[] pixels = new int[width * height];
        bitmap.getPixels(pixels, 0, width, 0, 0, width, height);
        return encodePixels(width, height, pixels);
    }

    static byte[] encodePixels(int width, int height, int[] pixels) {
        if (width <= 0 || height <= 0 || pixels.length != width * height) {
            throw new IllegalArgumentException("Pixel dimensions do not match the supplied data");
        }

        boolean hasTransparency = false;
        int cornerBrightness = 0;
        cornerBrightness += brightness(pixels[0]);
        cornerBrightness += brightness(pixels[width - 1]);
        cornerBrightness += brightness(pixels[(height - 1) * width]);
        cornerBrightness += brightness(pixels[height * width - 1]);
        cornerBrightness /= 4;
        boolean darkForeground = cornerBrightness >= 128;
        for (int pixel : pixels) {
            if (alpha(pixel) < 250) {
                hasTransparency = true;
                break;
            }
        }

        byte[] icon = new byte[BYTE_COUNT];
        for (int y = 0; y < SIZE; y++) {
            int sourceY = Math.min(height - 1, y * height / SIZE);
            for (int x = 0; x < SIZE; x++) {
                int sourceX = Math.min(width - 1, x * width / SIZE);
                int pixel = pixels[sourceY * width + sourceX];
                boolean foreground = hasTransparency
                        ? alpha(pixel) >= 96
                        : darkForeground
                                ? brightness(pixel) < 128
                                : brightness(pixel) >= 128;
                if (foreground) {
                    int offset = y * ROW_BYTES + x / 8;
                    icon[offset] |= (byte) (0x80 >> (x % 8));
                }
            }
        }
        return icon;
    }

    private static Bitmap getLargeIconBitmap(Notification notification) {
        Object extra = getParcelable(notification, Notification.EXTRA_LARGE_ICON);
        if (!(extra instanceof Bitmap)) {
            extra = getParcelable(notification, Notification.EXTRA_LARGE_ICON_BIG);
        }
        return extra instanceof Bitmap ? (Bitmap) extra : null;
    }

    private static Parcelable getParcelable(Notification notification, String key) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            return notification.extras.getParcelable(key, Parcelable.class);
        }
        return notification.extras.getParcelable(key);
    }

    private static int alpha(int color) {
        return (color >>> 24) & 0xFF;
    }

    private static int brightness(int color) {
        int red = (color >>> 16) & 0xFF;
        int green = (color >>> 8) & 0xFF;
        int blue = color & 0xFF;
        return (red * 299 + green * 587 + blue * 114)
                / 1000;
    }
}
