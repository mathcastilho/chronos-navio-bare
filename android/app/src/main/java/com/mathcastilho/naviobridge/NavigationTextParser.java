package com.mathcastilho.naviobridge;

import java.text.Normalizer;
import java.util.ArrayList;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Locale;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

final class NavigationTextParser {
    private static final Pattern DISTANCE = Pattern.compile(
            "(?i)(\\d+(?:[.,]\\d+)?)\\s*(km|kilometers?|kilometres?|mi|miles?|ft|feet|m|meters?|metres?)\\b");
    private static final Pattern DURATION = Pattern.compile(
            "(?i)(\\d+\\s*(?:h|hr|hrs|hour|hours)\\s*)?(\\d+\\s*(?:min|mins|minute|minutes))");
    private static final Pattern ETA = Pattern.compile(
            "(?i)\\b(?:[01]?\\d|2[0-3]):[0-5]\\d\\s*(?:a\\.?m\\.?|p\\.?m\\.?)?\\b");
    private static final Pattern SEPARATOR = Pattern.compile("[\\n\\r\\u2022\\u00b7|]+");
    private static final Pattern MANEUVER_WORDS = Pattern.compile(
            "(?i)\\b(turn|take|keep|continue|continue onto|merge|exit|ramp|roundabout|"
                    + "u-turn|uturn|arrive|head|slight|sharp|left|right|straight|"
                    + "gire|girar|tome|mantenha|continue|saia|rotatoria|"
                    + "gira|toma|mantente|sigue|sal|rotonda|"
                    + "tournez|prenez|continuez|restez|sortie|rond-point|"
                    + "biegen|nehmen|fahren|weiter|ausfahrt|kreisverkehr|"
                    + "svolta|prosegui|mantieni|esci|rotatoria)\\b");
    private static final Pattern DIRECTION_PREFIX = Pattern.compile(
            "(?i)\\b(?:in|within|after|en|dentro de|dentro|dans|nach)\\s+"
                    + "(\\d+(?:[.,]\\d+)?\\s*(?:km|kilometers?|kilometres?|mi|miles?|ft|feet|m|meters?|metres?))\\b");

    private NavigationTextParser() {
    }

    static NavigationText parse(List<String> notificationValues) {
        Set<String> segments = new LinkedHashSet<>();
        for (String value : notificationValues) {
            if (value == null) {
                continue;
            }
            for (String segment : SEPARATOR.split(value)) {
                String cleaned = segment.trim();
                if (!cleaned.isEmpty() && !isApplicationLabel(cleaned)) {
                    segments.add(cleaned);
                }
            }
        }

        String maneuverDistance = "";
        String fallbackDistance = "";
        String destinationDistance = "";
        String duration = "";
        String eta = "";
        List<String> allDistances = new ArrayList<>();
        ArrayList<String> instructionCandidates = new ArrayList<>();

        for (String segment : segments) {
            String lower = normalize(segment);
            Matcher distanceMatcher = DISTANCE.matcher(segment);
            List<String> distances = new ArrayList<>();
            while (distanceMatcher.find()) {
                String distance = formatDistance(
                        distanceMatcher.group(1), distanceMatcher.group(2));
                distances.add(distance);
                allDistances.add(distance);
                if (fallbackDistance.isEmpty()) {
                    fallbackDistance = distance;
                }
                if (hasDistancePrefix(lower, distanceMatcher.start())) {
                    maneuverDistance = distance;
                }
            }
            if (duration.isEmpty()) {
                Matcher durationMatcher = DURATION.matcher(segment);
                if (durationMatcher.find()) {
                    duration = normalizeDuration(durationMatcher);
                }
            }
            if (eta.isEmpty()) {
                Matcher etaMatcher = ETA.matcher(segment);
                if (etaMatcher.find()) {
                    eta = etaMatcher.group().trim().replaceAll("\\s+", " ");
                }
            }

            if (containsManeuver(segment)) {
                String instruction = DIRECTION_PREFIX.matcher(segment).replaceAll("").trim();
                instruction = removeMetricSummary(instruction);
                if (!instruction.isEmpty()) {
                    instructionCandidates.add(instruction);
                }
            } else if (distances.isEmpty() && !containsDuration(segment) && !containsEta(segment)) {
                instructionCandidates.add(segment);
            }
        }

        String directions = instructionCandidates.isEmpty()
                ? ""
                : instructionCandidates.get(0);
        if (allDistances.size() > 1) {
            destinationDistance = allDistances.get(allDistances.size() - 1);
        } else if (allDistances.size() == 1 && !duration.isEmpty()
                && maneuverDistance.isEmpty()) {
            destinationDistance = allDistances.get(0);
            fallbackDistance = "";
        }
        String title = maneuverDistance.isEmpty() ? fallbackDistance : maneuverDistance;
        String distance = destinationDistance;
        return new NavigationText(title, duration, distance, eta, directions, "");
    }

    private static boolean hasDistancePrefix(String normalized, int distanceStart) {
        String prefix = normalized.substring(0, Math.min(distanceStart, normalized.length()));
        return Pattern.compile(
                "(?i)(?:\\b(?:in|within|after|en|dentro de|dentro|dans|nach)\\s*)$")
                .matcher(prefix).find();
    }

    private static String formatDistance(String amount, String unit) {
        String value = amount.replace(',', '.').trim();
        String normalizedUnit = unit.toLowerCase(Locale.ROOT);
        if (normalizedUnit.equals("m") || normalizedUnit.startsWith("meter")
                || normalizedUnit.startsWith("metre")) {
            normalizedUnit = "m";
        } else if (normalizedUnit.equals("km") || normalizedUnit.startsWith("kilometer")
                || normalizedUnit.startsWith("kilometre")) {
            normalizedUnit = "km";
        } else if (normalizedUnit.equals("mi") || normalizedUnit.startsWith("mile")) {
            normalizedUnit = "mi";
        } else {
            normalizedUnit = "ft";
        }
        return value + " " + normalizedUnit;
    }

    private static String normalizeDuration(Matcher matcher) {
        String hours = matcher.group(1);
        String minutes = matcher.group(2);
        return ((hours == null ? "" : hours.trim() + " ") + minutes.trim())
                .replaceAll("\\s+", " ");
    }

    private static String removeMetricSummary(String text) {
        String withoutDistance = DISTANCE.matcher(text).replaceAll("").trim();
        return DURATION.matcher(withoutDistance).replaceAll("").trim();
    }

    private static boolean containsManeuver(String value) {
        return MANEUVER_WORDS.matcher(normalize(value)).find();
    }

    private static boolean containsDuration(String value) {
        return DURATION.matcher(value).find();
    }

    private static boolean containsEta(String value) {
        return ETA.matcher(value).find();
    }

    private static boolean isApplicationLabel(String value) {
        String normalized = normalize(value);
        return normalized.equals("google maps") || normalized.equals("maps")
                || normalized.equals("waze") || normalized.equals("osmand")
                || normalized.equals("navigation") || normalized.equals("directions");
    }

    private static String normalize(String value) {
        return Normalizer.normalize(value, Normalizer.Form.NFD)
                .replaceAll("\\p{M}+", "")
                .toLowerCase(Locale.ROOT);
    }
}
