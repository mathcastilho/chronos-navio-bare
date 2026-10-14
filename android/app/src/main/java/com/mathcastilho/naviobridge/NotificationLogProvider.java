package com.mathcastilho.naviobridge;

import android.content.ContentProvider;
import android.content.ContentValues;
import android.content.UriMatcher;
import android.database.Cursor;
import android.database.MatrixCursor;
import android.net.Uri;
import android.os.ParcelFileDescriptor;
import android.provider.OpenableColumns;

import java.io.File;
import java.io.FileNotFoundException;

public final class NotificationLogProvider extends ContentProvider {
    private static final int LOG_FILE = 1;
    private static final String FILE_NAME = "navigation-notifications.jsonl";
    private static final String MIME_TYPE = "text/plain";
    private UriMatcher uriMatcher;

    @Override
    public boolean onCreate() {
        uriMatcher = new UriMatcher(UriMatcher.NO_MATCH);
        uriMatcher.addURI(
                getContext().getPackageName() + ".notification-log",
                FILE_NAME,
                LOG_FILE);
        return true;
    }

    @Override
    public String getType(Uri uri) {
        return uriMatcher.match(uri) == LOG_FILE ? MIME_TYPE : null;
    }

    @Override
    public Cursor query(
            Uri uri, String[] projection, String selection,
            String[] selectionArgs, String sortOrder) {
        if (uriMatcher.match(uri) != LOG_FILE) {
            return null;
        }
        File file = logFile();
        MatrixCursor cursor = new MatrixCursor(
                new String[]{OpenableColumns.DISPLAY_NAME, OpenableColumns.SIZE});
        cursor.addRow(new Object[]{FILE_NAME, file.isFile() ? file.length() : 0});
        return cursor;
    }

    @Override
    public ParcelFileDescriptor openFile(Uri uri, String mode)
            throws FileNotFoundException {
        if (uriMatcher.match(uri) != LOG_FILE || !"r".equals(mode)) {
            throw new FileNotFoundException("Unsupported notification log URI or mode.");
        }
        File file = logFile();
        if (!file.isFile()) {
            throw new FileNotFoundException("No notification log has been recorded yet.");
        }
        return ParcelFileDescriptor.open(file, ParcelFileDescriptor.MODE_READ_ONLY);
    }

    @Override
    public Uri insert(Uri uri, ContentValues values) {
        throw new UnsupportedOperationException("Notification log is read-only.");
    }

    @Override
    public int delete(Uri uri, String selection, String[] selectionArgs) {
        throw new UnsupportedOperationException("Notification log is read-only.");
    }

    @Override
    public int update(
            Uri uri, ContentValues values, String selection, String[] selectionArgs) {
        throw new UnsupportedOperationException("Notification log is read-only.");
    }

    private File logFile() {
        return new File(getContext().getFilesDir(), FILE_NAME);
    }
}
