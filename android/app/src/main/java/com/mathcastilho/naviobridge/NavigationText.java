package com.mathcastilho.naviobridge;

import android.content.Intent;

final class NavigationText {
    static final String EXTRA_TITLE = "nav_title";
    static final String EXTRA_DURATION = "nav_duration";
    static final String EXTRA_DISTANCE = "nav_distance";
    static final String EXTRA_ETA = "nav_eta";
    static final String EXTRA_DIRECTIONS = "nav_directions";
    static final String EXTRA_SPEED = "nav_speed";
    static final String EXTRA_ICON = "nav_icon";

    final String title;
    final String duration;
    final String distance;
    final String eta;
    final String directions;
    final String speed;
    final byte[] icon;

    NavigationText(
            String title,
            String duration,
            String distance,
            String eta,
            String directions,
            String speed) {
        this(title, duration, distance, eta, directions, speed, null);
    }

    NavigationText(
            String title,
            String duration,
            String distance,
            String eta,
            String directions,
            String speed,
            byte[] icon) {
        this.title = nonNull(title);
        this.duration = nonNull(duration);
        this.distance = nonNull(distance);
        this.eta = nonNull(eta);
        this.directions = nonNull(directions);
        this.speed = nonNull(speed);
        this.icon = icon == null ? null : icon.clone();
    }

    static NavigationText fromIntent(Intent intent) {
        return new NavigationText(
                intent.getStringExtra(EXTRA_TITLE),
                intent.getStringExtra(EXTRA_DURATION),
                intent.getStringExtra(EXTRA_DISTANCE),
                intent.getStringExtra(EXTRA_ETA),
                intent.getStringExtra(EXTRA_DIRECTIONS),
                intent.getStringExtra(EXTRA_SPEED),
                intent.getByteArrayExtra(EXTRA_ICON));
    }

    boolean hasContent() {
        return !title.isEmpty() || !duration.isEmpty() || !distance.isEmpty()
                || !eta.isEmpty() || !directions.isEmpty() || !speed.isEmpty();
    }

    private static String nonNull(String value) {
        return value == null ? "" : value.trim();
    }
}
