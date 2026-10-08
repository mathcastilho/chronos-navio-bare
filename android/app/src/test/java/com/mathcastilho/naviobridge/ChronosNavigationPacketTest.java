package com.mathcastilho.naviobridge;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import org.junit.Test;

public final class ChronosNavigationPacketTest {
    @Test
    public void inactivePacketUsesChronosNavigationStopFrame() {
        byte[] packet = ChronosNavigationPacket.inactive();

        assertEquals(6, packet.length);
        assertEquals(0xAB, packet[0] & 0xFF);
        assertEquals(3, packet[2] & 0xFF);
        assertEquals(0xFE, packet[3] & 0xFF);
        assertEquals(0xEF, packet[4] & 0xFF);
        assertEquals(0, packet[5] & 0xFF);
    }

    @Test
    public void activePacketContainsNullTerminatedDirectionAndValidLength() {
        String directions = "Turn left onto Main Street";
        byte[] packet = ChronosNavigationPacket.active(directions);
        int protocolLength = ((packet[1] & 0xFF) << 8) | (packet[2] & 0xFF);

        assertEquals(packet.length - 3, protocolLength);
        assertEquals(0xFE, packet[3] & 0xFF);
        assertEquals(0xEF, packet[4] & 0xFF);
        assertEquals(0x80, packet[5] & 0xFF);
        assertEquals(1, packet[7] & 0xFF);
        assertTrue(new String(packet, StandardCharsets.UTF_8).contains(directions));
        assertEquals(0, packet[packet.length - 1]);
    }

    @Test
    public void activePacketsSendThreeIconChunksBeforeNavigationData() {
        List<byte[]> packets =
                ChronosNavigationPacket.activePackets("Turn left onto Main Street");

        assertEquals(4, packets.size());
        byte[] assembledIcon = new byte[288];
        long iconCrc = 0;
        for (int chunk = 0; chunk < 3; chunk++) {
            byte[] packet = packets.get(chunk);
            assertEquals(107, packet.length);
            assertEquals(0xAB, packet[0] & 0xFF);
            assertEquals(104, packet[2] & 0xFF);
            assertEquals(0xFE, packet[3] & 0xFF);
            assertEquals(0xEE, packet[4] & 0xFF);
            assertEquals(chunk, packet[6] & 0xFF);
            System.arraycopy(packet, 11, assembledIcon, chunk * 96, 96);
            long chunkCrc = readUnsignedInt(packet, 7);
            if (chunk == 0) {
                iconCrc = chunkCrc;
            } else {
                assertEquals(iconCrc, chunkCrc);
            }
        }

        byte[] activePacket = packets.get(3);
        assertEquals(0xEF, activePacket[4] & 0xFF);
        assertEquals(1, activePacket[6] & 0xFF);
        assertEquals(1, activePacket[7] & 0xFF);
        assertEquals(iconCrc, readUnsignedInt(activePacket, 8));
        assertTrue(containsNonZero(assembledIcon));
    }

    @Test
    public void encodesNotificationGraphicAsMonochromeA1() {
        int[] pixels = new int[48 * 48];
        pixels[0] = 0xFFFFFFFF;
        pixels[7] = 0xFFFFFFFF;

        byte[] icon = NavigationIcon.encodePixels(48, 48, pixels);

        assertEquals(0x81, icon[0] & 0xFF);
        assertEquals(0, icon[1] & 0xFF);
        assertEquals(NavigationIcon.BYTE_COUNT, icon.length);
    }

    @Test
    public void notificationGraphicOverridesGeneratedManeuverGlyph() {
        byte[] notificationIcon = new byte[NavigationIcon.BYTE_COUNT];
        notificationIcon[0] = (byte) 0xA5;
        NavigationText navigationText = new NavigationText(
                "", "", "", "", "Turn right", "", notificationIcon);

        List<byte[]> packets = ChronosNavigationPacket.activePackets(navigationText);

        assertEquals(0xA5, packets.get(0)[11] & 0xFF);
        assertEquals(0, packets.get(0)[12] & 0xFF);
    }

    @Test
    public void deviceSettingsUseVersionedChronosFrameWithDefaultValues() {
        byte[] packet = new DeviceSettings().toPacket();

        assertEquals(19, packet.length);
        assertEquals(0xAB, packet[0] & 0xFF);
        assertEquals(16, packet[2] & 0xFF);
        assertEquals(0xFE, packet[3] & 0xFF);
        assertEquals(0x7E, packet[4] & 0xFF);
        assertEquals(3, packet[5] & 0xFF);
        assertEquals(100, packet[6] & 0xFF);
        assertEquals(3, packet[7] & 0xFF);
        assertEquals(384, ((packet[8] & 0xFF) << 8) | (packet[9] & 0xFF));
        assertEquals(0, packet[10] & 0xFF);
        assertEquals(1, packet[11] & 0xFF);
        assertEquals(1, packet[12] & 0xFF);
        assertEquals(0xFFFFFF, readUnsignedInt(packet, 13));
        assertEquals(1, packet[17] & 0xFF);
        assertEquals(0, packet[18] & 0xFF);
    }

    @Test
    public void deviceSettingsSerializeTripVisibilityAndHeadingPosition() {
        DeviceSettings settings = new DeviceSettings();
        settings.showTripInfo = false;
        settings.showEta = true;
        settings.headingAboveIcon = true;

        byte[] packet = settings.toPacket();

        assertEquals(1, packet[10] & 0xFF);
        assertEquals(0, packet[17] & 0xFF);
        assertEquals(1, packet[18] & 0xFF);
    }

    @Test
    public void maneuverTextSelectsDirectionArrow() {
        assertEquals("left", ChronosNavigationPacket.maneuverFor("Turn left onto Main Street"));
        assertEquals("right", ChronosNavigationPacket.maneuverFor("Gire a la derecha"));
        assertEquals("uturn", ChronosNavigationPacket.maneuverFor("Make a U-turn"));
        assertEquals("straight", ChronosNavigationPacket.maneuverFor("Continue for 2 km"));
    }

    @Test
    public void parsesInstructionAndTripSummaryFromMapsNotification() {
        NavigationText text = NavigationTextParser.parse(Arrays.asList(
                "Google Maps",
                "In 500 m turn left onto Rua das Flores",
                "6 min · 2.4 km · 15:42"));

        assertEquals("500 m", text.title);
        assertEquals("6 min", text.duration);
        assertEquals("2.4 km", text.distance);
        assertEquals("15:42", text.eta);
        assertEquals("turn left onto Rua das Flores", text.directions);
        assertEquals("", text.speed);
    }

    @Test
    public void preservesPortugueseInstructionWhenNoTripMetricsAreAvailable() {
        NavigationText text = NavigationTextParser.parse(Arrays.asList(
                "Google Maps", "Vire à direita na Avenida Paulista"));

        assertEquals("Vire à direita na Avenida Paulista", text.directions);
        assertEquals("", text.title);
        assertEquals("", text.duration);
        assertEquals("", text.distance);
        assertEquals("", text.eta);
    }

    @Test
    public void doesNotTreatOnlyTripDistanceAsNextManeuverDistance() {
        NavigationText text = NavigationTextParser.parse(Arrays.asList(
                "Continue on Main Street", "6 min · 2.4 km"));

        assertEquals("6 min", text.duration);
        assertEquals("2.4 km", text.distance);
        assertEquals("", text.title);
        assertEquals("Continue on Main Street", text.directions);
    }

    @Test
    public void activePacketSerializesAllSixChronosTextFieldsInOrder() {
        NavigationText text = new NavigationText(
                "500 m", "6 min", "2.4 km", "15:42",
                "Turn left onto Rua das Flores", "32 km/h");
        byte[] packet = ChronosNavigationPacket.activePackets(text).get(3);

        assertEquals(Arrays.asList(
                "500 m", "6 min", "2.4 km", "15:42",
                "Turn left onto Rua das Flores", "32 km/h"),
                readNavigationStrings(packet));
    }

    private List<String> readNavigationStrings(byte[] packet) {
        ArrayList<String> values = new ArrayList<>();
        int start = 12;
        for (int field = 0; field < 6; field++) {
            int end = start;
            while (packet[end] != 0) {
                end++;
            }
            values.add(new String(packet, start, end - start, StandardCharsets.UTF_8));
            start = end + 1;
        }
        return values;
    }

    private long readUnsignedInt(byte[] bytes, int offset) {
        return ((long) (bytes[offset] & 0xFF) << 24)
                | ((long) (bytes[offset + 1] & 0xFF) << 16)
                | ((long) (bytes[offset + 2] & 0xFF) << 8)
                | (long) (bytes[offset + 3] & 0xFF);
    }

    private boolean containsNonZero(byte[] bytes) {
        for (byte value : bytes) {
            if (value != 0) {
                return true;
            }
        }
        return false;
    }
}
