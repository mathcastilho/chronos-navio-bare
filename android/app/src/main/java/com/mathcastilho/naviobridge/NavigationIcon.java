package com.mathcastilho.naviobridge;

import android.app.Notification;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.drawable.Drawable;
import android.os.Build;
import android.os.Parcelable;
import android.graphics.drawable.Icon;

final class NavigationIcon {
    static final int SIZE = 48;
    static final int ROW_BYTES = SIZE / 8;
    static final int BYTE_COUNT = SIZE * ROW_BYTES;
    private static final int[] BAYER_4X4 = {
            0, 8, 2, 10,
            12, 4, 14, 6,
            3, 11, 1, 9,
            15, 7, 13, 5
    };
    private static final int PREVIEW_SCALE = 3;

    private NavigationIcon() {
    }

    static byte[] fromNotification(Context context, Notification notification) {
        int ditheringLevel = DeviceSettings.load(context).iconDitheringLevel;
        Bitmap bitmap = getLargeIconBitmap(notification);
        if (bitmap != null) {
            return fromBitmap(bitmap, ditheringLevel);
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
        return fromBitmap(rendered, ditheringLevel);
    }

    static byte[] fromBitmap(Bitmap bitmap) {
        return fromBitmap(bitmap, 0);
    }

    static byte[] fromBitmap(Bitmap bitmap, int ditheringLevel) {
        int width = bitmap.getWidth();
        int height = bitmap.getHeight();
        int[] pixels = new int[width * height];
        bitmap.getPixels(pixels, 0, width, 0, 0, width, height);
        return encodePixels(width, height, pixels, ditheringLevel);
    }

    static byte[] encodePixels(int width, int height, int[] pixels) {
        return encodePixels(width, height, pixels, 0);
    }

    static byte[] encodePixels(int width, int height, int[] pixels, int ditheringLevel) {
        if (width <= 0 || height <= 0 || pixels.length != width * height) {
            throw new IllegalArgumentException("Pixel dimensions do not match the supplied data");
        }
        int level = Math.max(0, Math.min(
                ditheringLevel, DeviceSettings.ICON_DITHERING_LEVEL_COUNT - 1));

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
                int pixelBrightness = brightness(pixel);
                boolean foreground = hasTransparency
                        ? alpha(pixel) >= 96
                        : darkForeground
                        ? pixelBrightness < 128
                        : pixelBrightness >= 128;
                if (level > 0) {
                    int sourceCoverage = hasTransparency
                            ? alpha(pixel)
                            : darkForeground ? 255 - pixelBrightness : pixelBrightness;
                    int legacyCoverage = foreground ? 255 : 0;
                    int coverage = (legacyCoverage * (3 - level) + sourceCoverage * level) / 3;
                    int threshold = BAYER_4X4[(y & 3) * 4 + (x & 3)] * 16 + 8;
                    foreground = coverage > threshold;
                }
                if (foreground) {
                    int offset = y * ROW_BYTES + x / 8;
                    icon[offset] |= (byte) (0x80 >> (x % 8));
                }
            }
        }
        return icon;
    }

    static Bitmap createDitheringPreview(int ditheringLevel) {
        Bitmap sample = Bitmap.createBitmap(SIZE, SIZE, Bitmap.Config.ARGB_8888);
        Canvas sampleCanvas = new Canvas(sample);
        Paint paint = new Paint(Paint.ANTI_ALIAS_FLAG);
        paint.setColor(0x80FFFFFF);
        paint.setStyle(Paint.Style.STROKE);
        paint.setStrokeWidth(5.5f);
        sampleCanvas.drawCircle(18, 19, 10, paint);

        paint.setColor(Color.WHITE);
        paint.setStrokeWidth(6);
        paint.setStrokeCap(Paint.Cap.ROUND);
        paint.setStrokeJoin(Paint.Join.ROUND);
        Path route = new Path();
        route.moveTo(18, 46);
        route.lineTo(18, 29);
        route.quadTo(18, 19, 29, 19);
        route.lineTo(41, 19);
        sampleCanvas.drawPath(route, paint);
        Path arrowHead = new Path();
        arrowHead.moveTo(35, 13);
        arrowHead.lineTo(41, 19);
        arrowHead.lineTo(35, 25);
        sampleCanvas.drawPath(arrowHead, paint);

        byte[] encoded = fromBitmap(sample, ditheringLevel);
        sample.recycle();
        Bitmap preview = Bitmap.createBitmap(
                SIZE * PREVIEW_SCALE, SIZE * PREVIEW_SCALE, Bitmap.Config.ARGB_8888);
        preview.eraseColor(Color.BLACK);
        for (int y = 0; y < SIZE; y++) {
            for (int x = 0; x < SIZE; x++) {
                int offset = y * ROW_BYTES + x / 8;
                if ((encoded[offset] & (0x80 >> (x % 8))) != 0) {
                    for (int dy = 0; dy < PREVIEW_SCALE; dy++) {
                        for (int dx = 0; dx < PREVIEW_SCALE; dx++) {
                            preview.setPixel(
                                    x * PREVIEW_SCALE + dx,
                                    y * PREVIEW_SCALE + dy,
                                    Color.WHITE);
                        }
                    }
                }
            }
        }
        return preview;
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
