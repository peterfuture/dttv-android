package dttv.app;

import android.content.Context;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteCursor;
import android.database.sqlite.SQLiteCursorDriver;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.database.sqlite.SQLiteQuery;
import android.database.sqlite.SQLiteStatement;
import android.util.Log;

public class FileBrowserDatabase extends SQLiteOpenHelper {
    /**
     * The name of the database file on the file system
     */
    private static final String DATABASE_NAME = "FileBrowserDatabase";
    /**
     * The version of the database that this class understands.
     */
    private static final int DATABASE_VERSION = 2;
    /**
     * Keep track of context so that we can load SQL from string resources
     */
    private final Context mContext;

    public static final String TAG = "FileBrowserDatabase";

    /**
     * Constructor
     */
    public FileBrowserDatabase(Context context) {
        super(context, DATABASE_NAME, null, DATABASE_VERSION);
        this.mContext = context;
    }

    /**
     * Execute all of the SQL statements in the String[] array
     *
     * @param db  The database on which to execute the statements
     * @param sql An array of SQL statements to execute
     */
    private void execMultipleSQL(SQLiteDatabase db, String[] sql) {
        for (String s : sql)
            if (s.trim().length() > 0)
                db.execSQL(s);

    }

    /**
     * Called when it is time to create the database
     */
    @Override
    public void onCreate(SQLiteDatabase db) {
        String[] sql = mContext.getString(R.string.FileBrowserDatabase_onCreate).split("\n");
        db.beginTransaction();
        try {
            // Create tables & test data
            execMultipleSQL(db, sql);
            db.setTransactionSuccessful();
        } catch (SQLException e) {
            Log.e(TAG, e.toString());
        } finally {
            db.endTransaction();
        }
    }

    /**
     * Called when the database must be upgraded
     */
    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        Log.w(FileBrowserActivity.TAG, "Upgrading database from version " + oldVersion + " to " +
                newVersion + ", which will destroy all old data");

        String[] sql = mContext.getString(R.string.FileBrowserDatabase_onUpgrade).split("\n");
        db.beginTransaction();
        try {
            // Create tables & test data
            execMultipleSQL(db, sql);
            db.setTransactionSuccessful();
        } catch (SQLException e) {
            Log.e(TAG, e.toString());
        } finally {
            db.endTransaction();
        }

        // This is cheating.  In the real world, you'll need to add columns, not rebuild from scratch
        onCreate(db);
    }

    /**
     * Provides self-contained query-specific cursor for file_mark_table.
     * The query and all accessor methods are in the class.
     */
    public static class FileMarkCursor extends SQLiteCursor {
        /**
         * The query for this cursor
         */
        private static final String QUERY =
                "SELECT _id, file_path, is_sel " +
                        "FROM file_mark_table ";

        /**
         * Cursor constructor
         */
        private FileMarkCursor(SQLiteDatabase db, SQLiteCursorDriver driver,
                               String editTable, SQLiteQuery query) {
            super(db, driver, editTable, query);
        }

        /**
         * Private factory class necessary for rawQueryWithFactory() call
         */
        private static class Factory implements SQLiteDatabase.CursorFactory {

            public Cursor newCursor(SQLiteDatabase db,
                                    SQLiteCursorDriver driver, String editTable,
                                    SQLiteQuery query) {
                return new FileMarkCursor(db, driver, editTable, query);
            }
        }

        /* Accessor functions -- one per database column */
        public long getColId() {
            return getLong(getColumnIndexOrThrow("_id"));
        }

        public String getColFilePath() {
            return getString(getColumnIndexOrThrow("file_path"));
        }

        public long getColIsSel() {
            return getLong(getColumnIndexOrThrow("is_sel"));
        }
    }

    /**
     * Returns a Cursor for file_mark_table
     */
    public FileMarkCursor getFileMark() {
        SQLiteDatabase d = getReadableDatabase();
        FileMarkCursor c = (FileMarkCursor) d.rawQueryWithFactory(
                new FileMarkCursor.Factory(),
                FileMarkCursor.QUERY,
                null,
                null);
        c.moveToFirst();
        return c;
    }

    /**
     * Returns a Cursor for file_mark_table query by file_path
     */
    public FileMarkCursor getFileMarkByPath(String file_path) {
        file_path = file_path.replace("'", "''");
        String sql = String.format(
                FileMarkCursor.QUERY +
                        "WHERE file_path = '%s' ",
                file_path);
        SQLiteDatabase d = getReadableDatabase();
        FileMarkCursor c = (FileMarkCursor) d.rawQueryWithFactory(
                new FileMarkCursor.Factory(),
                sql,
                null,
                null);
        c.moveToFirst();
        return c;
    }

    /**
     * add entry to database
     */
    public void addFileMark(String file_path, int is_sel) {
        //Log.w(FileBrower.TAG, "addFileMark: " + file_path);
        file_path = file_path.replace("'", "''");
        String sql = String.format(
                "INSERT INTO file_mark_table (_id, file_path, is_sel) " +
                        "VALUES (NULL, '%s', '%d')",
                file_path, is_sel);
        try {
            getWritableDatabase().execSQL(sql);
        } catch (SQLException e) {
            Log.e(TAG, e.toString());
        }
    }

    /**
     * delete entry
     */
    public void deleteFileMark(String file_path) {
        //Log.w(FileBrower.TAG, "deleteFileMark: " + file_path);
        file_path = file_path.replace("'", "''");
        String sql = String.format(
                "DELETE FROM file_mark_table " +
                        "WHERE file_path = '%s' ",
                file_path);
        try {
            getWritableDatabase().execSQL(sql);
        } catch (SQLException e) {
            Log.e(TAG, e.toString());
        }
    }

    /**
     * delete all entry
     */
    public void deleteAllFileMark() {
        //Log.w(FileBrower.TAG, "deleteAllFileMark: ");
        String sql = String.format(
                "DELETE FROM file_mark_table "
        );
        try {
            getWritableDatabase().execSQL(sql);
        } catch (SQLException e) {
            Log.e(TAG, e.toString());
        }
    }

    /**
     * update entry
     */
    public void updateFileMark(String file_path, int is_sel) {
        //Log.w(FileBrower.TAG, "updateFileMark: " + file_path);
        file_path = file_path.replace("'", "''");
        String sql = String.format(
                "UPDATE file_mark_table " +
                        "SET is_sel = '%d', " +
                        "WHERE file_path = '%s' ",
                is_sel, file_path);
        try {
            getWritableDatabase().execSQL(sql);
        } catch (SQLException e) {
            Log.e(TAG, e.toString());
        }
    }


    /**
     * Provides self-contained query-specific cursor for file_thumbnails_table.
     * The query and all accessor methods are in the class.
     */
    public static class ThumbnailCursor extends SQLiteCursor {
        /**
         * The query for this cursor
         */
        private static final String QUERY =
                "SELECT _id, file_path, file_data " +
                        "FROM file_thumbnails_table ";

        /**
         * Cursor constructor
         */
        private ThumbnailCursor(SQLiteDatabase db, SQLiteCursorDriver driver,
                                String editTable, SQLiteQuery query) {
            super(db, driver, editTable, query);
        }

        /**
         * Private factory class necessary for rawQueryWithFactory() call
         */
        private static class Factory implements SQLiteDatabase.CursorFactory {

            public Cursor newCursor(SQLiteDatabase db,
                                    SQLiteCursorDriver driver, String editTable,
                                    SQLiteQuery query) {
                return new ThumbnailCursor(db, driver, editTable, query);
            }
        }

        /* Accessor functions -- one per database column */
        public long getColId() {
            return getLong(getColumnIndexOrThrow("_id"));
        }

        public String getColFilePath() {
            return getString(getColumnIndexOrThrow("file_path"));
        }

        public byte[] getColFileData() {
            return getBlob(getColumnIndexOrThrow("file_data"));
        }
    }

    /**
     * Returns a Cursor for file_thumbnails_table query by file_path
     */
    public ThumbnailCursor getThumbnailByPath(String file_path) {
        file_path = file_path.replace("'", "''");
        String sql = String.format(
                ThumbnailCursor.QUERY +
                        "WHERE file_path = '%s' ",
                file_path);
        SQLiteDatabase d = getReadableDatabase();
        ThumbnailCursor c = (ThumbnailCursor) d.rawQueryWithFactory(
                new ThumbnailCursor.Factory(),
                sql,
                null,
                null);
        c.moveToFirst();
        return c;
    }

    public ThumbnailCursor checkThumbnailByPath(String file_path) {
        file_path = file_path.replace("'", "''");
        String sql = String.format(
                "SELECT _id, file_path " +
                        "FROM file_thumbnails_table " +
                        "WHERE file_path = '%s' ",
                file_path);
        SQLiteDatabase d = getReadableDatabase();
        ThumbnailCursor c = (ThumbnailCursor) d.rawQueryWithFactory(
                new ThumbnailCursor.Factory(),
                sql,
                null,
                null);
        c.moveToFirst();
        return c;
    }

    public ThumbnailCursor checkThumbnail() {
        String sql =
                "SELECT _id, file_path " +
                        "FROM file_thumbnails_table ";
        SQLiteDatabase d = getReadableDatabase();
        ThumbnailCursor c = (ThumbnailCursor) d.rawQueryWithFactory(
                new ThumbnailCursor.Factory(),
                sql,
                null,
                null);
        c.moveToFirst();
        return c;
    }

    /**
     * add entry to database
     */
    public void addThumbnail(String file_path, byte[] file_data) {
        //Log.w(FileBrower.TAG, "addThumbnail: " + file_path);


        try {
            SQLiteStatement statement = null;
            try {
                statement = getWritableDatabase()
                        .compileStatement(
                                "INSERT INTO file_thumbnails_table (_id, file_path, file_data) " +
                                        "VALUES (NULL, ?, ?)");

                statement.bindString(1, file_path);
                statement.bindBlob(2, file_data);
                statement.execute();
            } finally {
                if (statement != null) statement.close();
            }

        } catch (SQLException e) {
            Log.e(TAG, e.toString());
        }
    }

    /**
     * delete entry
     */
    public void deleteThumbnail(String file_path) {
        //Log.w(FileBrower.TAG, "deleteThumbnail: " + file_path);
        file_path = file_path.replace("'", "''");
        String sql = String.format(
                "DELETE FROM file_thumbnails_table " +
                        "WHERE file_path = '%s' ",
                file_path);
        try {
            getWritableDatabase().execSQL(sql);
        } catch (SQLException e) {
            Log.e(TAG, e.toString());
        }
    }

    /**
     * delete all entry
     */
    public void deleteAllThumbnail() {
        //Log.w(FileBrower.TAG, "deleteAllThumbnail: ");
        String sql = String.format(
                "DELETE FROM file_thumbnails_table "
        );
        try {
            getWritableDatabase().execSQL(sql);
        } catch (SQLException e) {
            Log.e(TAG, e.toString());
        }
    }
}
