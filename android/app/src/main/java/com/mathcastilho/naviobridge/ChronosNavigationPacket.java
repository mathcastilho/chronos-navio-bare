package com.mathcastilho.naviobridge;

import java.io.ByteArrayOutputStream;
import java.text.Normalizer;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
import java.util.zip.CRC32;

final class ChronosNavigationPacket {
    private static final int MAX_PACKET_SIZE = 512;
    private static final int MAX_DIRECTION_BYTES = 180;
    private static final int ICON_SIZE = NavigationIcon.SIZE;
    private static final int ICON_ROW_BYTES = NavigationIcon.ROW_BYTES;
    private static final int ICON_CHUNK_SIZE = 96;

    private ChronosNavigationPacket() {
    }

    static byte[] active(String directions) {
        return active(new NavigationText("", "", "", "", directions, ""));
    }

    static List<byte[]> activePackets(String directions) {
        return activePackets(new NavigationText("", "", "", "", directions, ""));
    }

    static List<byte[]> activePackets(NavigationText navigationText) {
        byte[] icon = navigationText.icon != null
                && navigationText.icon.length == NavigationIcon.BYTE_COUNT
                ? navigationText.icon.clone()
                : createManeuverIcon(navigationText.directions);
        long iconCrc = crc32(icon);
        ArrayList<byte[]> packets = new ArrayList<>(4);
        for (int chunk = 0; chunk < 3; chunk++) {
            ByteArrayOutputStream payload = new ByteArrayOutputStream(104);
            payload.write(0xFE);
            payload.write(0xEE);
            payload.write(0x00);
            payload.write(chunk);
            writeCrc(payload, iconCrc);
            payload.write(icon, chunk * ICON_CHUNK_SIZE, ICON_CHUNK_SIZE);
            packets.add(frame(payload.toByteArray()));
        }
        packets.add(active(navigationText, iconCrc));
        return packets;
    }

    static byte[] inactive() {
        return frame(new byte[]{(byte) 0xFE, (byte) 0xEF, 0x00});
    }

    static String maneuverFor(String directions) {
        String normalized = Normalizer.normalize(directions, Normalizer.Form.NFD)
                .replaceAll("\\p{M}+", "")
                .toLowerCase(Locale.ROOT);
        if (containsAny(normalized, "u-turn", "uturn", "u turn", "turn around",
                "hacer un cambio de sentido", "cambio de sentido", "retorno", "retournez")) {
            return "uturn";
        }
        if (containsAny(normalized, "left", "izquierda", "esquerda", "gauche",
                "links", "sinistra", "向左", "左转")) {
            return "left";
        }
        if (containsAny(normalized, "right", "derecha", "direita", "droite",
                "rechts", "destra", "向右", "右转")) {
            return "right";
        }
        return "straight";
    }

    private static byte[] frame(byte[] payload) {
        int totalLength = payload.length + 3;
        if (totalLength > MAX_PACKET_SIZE) {
            throw new IllegalArgumentException("Chronos packet exceeds the ESP32 receive buffer");
        }

        byte[] packet = new byte[totalLength];
        packet[0] = (byte) 0xAB;
        packet[1] = (byte) ((payload.length >> 8) & 0xFF);
        packet[2] = (byte) (payload.length & 0xFF);
        System.arraycopy(payload, 0, packet, 3, payload.length);
        return packet;
    }

    private static byte[] active(NavigationText navigationText) {
        byte[] icon = createManeuverIcon(navigationText.directions);
        return active(navigationText, crc32(icon));
    }

    private static byte[] active(NavigationText navigationText, long iconCrc) {
        ByteArrayOutputStream payload = new ByteArrayOutputStream();
        payload.write(0xFE);
        payload.write(0xEF);
        payload.write(0x80);
        payload.write(0x01);
        payload.write(0x01);
        writeCrc(payload, iconCrc);

        writeString(payload, truncateUtf8(navigationText.title, 48));
        writeString(payload, truncateUtf8(navigationText.duration, 48));
        writeString(payload, truncateUtf8(navigationText.distance, 48));
        writeString(payload, truncateUtf8(navigationText.eta, 40));
        writeString(payload, truncateUtf8(navigationText.directions, MAX_DIRECTION_BYTES));
        writeString(payload, truncateUtf8(navigationText.speed, 24));
        return frame(payload.toByteArray());
    }

    private static byte[] createManeuverIcon(String directions) {
        byte[] icon = new byte[ICON_SIZE * ICON_ROW_BYTES];
        String maneuver = maneuverFor(directions);
        if ("left".equals(maneuver) || "uturn".equals(maneuver)) {
            drawLeftArrow(icon);
        } else if ("right".equals(maneuver)) {
            drawRightArrow(icon);
        } else {
            drawUpArrow(icon);
        }
        return icon;
    }

    private static void drawUpArrow(byte[] icon) {
        fillRect(icon, 21, 18, 27, 42);
        for (int y = 5; y <= 24; y++) {
            int halfWidth = (y - 5) / 2;
            fillRect(icon, 24 - halfWidth, y, 24 + halfWidth, y);
        }
    }

    private static void drawLeftArrow(byte[] icon) {
        fillRect(icon, 23, 16, 29, 42);
        fillRect(icon, 13, 10, 26, 18);
        for (int x = 4; x <= 18; x++) {
            int halfHeight = Math.min(11, (x - 4) * 3 / 2);
            fillRect(icon, x, 14 - halfHeight, x, 14 + halfHeight);
        }
    }

    private static void drawRightArrow(byte[] icon) {
        fillRect(icon, 19, 16, 25, 42);
        fillRect(icon, 21, 10, 34, 18);
        for (int x = 29; x <= 43; x++) {
            int halfHeight = Math.min(11, (43 - x) * 3 / 2);
            fillRect(icon, x, 14 - halfHeight, x, 14 + halfHeight);
        }
    }

    private static void fillRect(byte[] icon, int left, int top, int right, int bottom) {
        for (int y = Math.max(0, top); y <= Math.min(ICON_SIZE - 1, bottom); y++) {
            for (int x = Math.max(0, left); x <= Math.min(ICON_SIZE - 1, right); x++) {
                int offset = y * ICON_ROW_BYTES + x / 8;
                icon[offset] |= (byte) (0x80 >> (x % 8));
            }
        }
    }

    private static boolean containsAny(String value, String... needles) {
        for (String needle : needles) {
            if (value.contains(needle)) {
                return true;
            }
        }
        return false;
    }

    private static long crc32(byte[] bytes) {
        CRC32 crc = new CRC32();
        crc.update(bytes);
        return crc.getValue() & 0x7FFFFFFFL;
    }

    private static void writeCrc(ByteArrayOutputStream output, long crc) {
        output.write((int) ((crc >>> 24) & 0xFF));
        output.write((int) ((crc >>> 16) & 0xFF));
        output.write((int) ((crc >>> 8) & 0xFF));
        output.write((int) (crc & 0xFF));
    }

    private static void writeString(ByteArrayOutputStream output, String value) {
        byte[] bytes = value.getBytes(StandardCharsets.UTF_8);
        output.write(bytes, 0, bytes.length);
        output.write(0);
    }

    private static String truncateUtf8(String value, int maxBytes) {
        StringBuilder result = new StringBuilder();
        int byteCount = 0;
        for (int offset = 0; offset < value.length();) {
            int codePoint = value.codePointAt(offset);
            String next = new String(Character.toChars(codePoint));
            int nextBytes = next.getBytes(StandardCharsets.UTF_8).length;
            if (byteCount + nextBytes > maxBytes) {
                break;
            }
            result.append(next);
            byteCount += nextBytes;
            offset += Character.charCount(codePoint);
        }
        return result.toString();
    }
}
