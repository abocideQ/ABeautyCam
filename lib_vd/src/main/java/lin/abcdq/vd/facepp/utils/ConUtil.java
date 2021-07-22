package lin.abcdq.vd.facepp.utils;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.media.ExifInterface;
import android.net.Uri;
import android.os.Build;
import android.os.Environment;
import android.os.PowerManager;
import android.provider.MediaStore;
import android.util.Base64;
import android.util.Log;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.util.UUID;

public class ConUtil {
    public static final int SEGMENT_THREAD_COUNT = 1; //抠像视频流检测使用线程数
    public static final boolean SEGMENT_IS_SMOOTH = false; //抠像是否开启平滑


    public static String getUUIDString(Context mContext) {
        String KEY_UUID = "key_uuid";
        SharedUtil sharedUtil = new SharedUtil(mContext);
        String uuid = sharedUtil.getStringValueByKey(KEY_UUID);
        if (uuid != null && uuid.trim().length() != 0)
            return uuid;

        uuid = UUID.randomUUID().toString();
        uuid = Base64.encodeToString(uuid.getBytes(),
                Base64.DEFAULT);

        sharedUtil.saveStringValue(KEY_UUID, uuid);
        return uuid;
    }

    public static byte[] readAssetsData(Context context, String fileName) {
        try {
            InputStream is = context.getAssets().open(fileName);
            int lenght = is.available();
            byte[] buffer = new byte[lenght];
            is.read(buffer);
            is.close();
            return buffer;
        } catch (IOException e) {
            e.printStackTrace();
        }
        return null;
    }

    public static void getAppDetailSettingIntent(Activity context) {
        Intent intent = new Intent();
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        if (Build.VERSION.SDK_INT >= 9) {
            intent.setAction("android.settings.APPLICATION_DETAILS_SETTINGS");
            intent.setData(Uri.fromParts("package", context.getPackageName(), null));
        } else if (Build.VERSION.SDK_INT <= 8) {
            intent.setAction(Intent.ACTION_VIEW);
            intent.setClassName("com.android.settings", "com.android.settings.InstalledAppDetails");
            intent.putExtra("com.android.settings.ApplicationPkgName", context.getPackageName());
        }
        context.startActivity(intent);
    }

    public static String getRealPathFromURI(Context context, Uri contentURI) {
        Log.e("initData","uri:"+contentURI.toString());
        Log.e("initData","uri.path:"+contentURI.getPath());
        String result;
        Cursor cursor = context.getContentResolver().query(contentURI, null, null, null, null);
        if (cursor == null) {
            // Source is Dropbox or other similar local file path
            result = contentURI.getPath();
        } else {
            cursor.moveToFirst();
            int idx = cursor.getColumnIndex(MediaStore.Images.ImageColumns.DATA);
            result = cursor.getString(idx);
            cursor.close();
        }
        return result;
    }

    public static String getVideoRealPathFromURI(Context context, Uri selectedUri) {
        Log.e("initData","uri:"+selectedUri.toString());
        Log.e("initData","uri.path:"+selectedUri.getPath());
        if (isQQMediaDocument(selectedUri)) {
            String path = selectedUri.getPath();
            File fileDir = Environment.getExternalStorageDirectory();
            File file = new File(fileDir, path.substring("/QQBrowser".length(), path.length()));
            return file.exists() ? file.toString() : null;
        }
        String contentPath;
        String[] columns = {MediaStore.Images.Media.DATA,
                MediaStore.Images.Media.MIME_TYPE};
        Cursor cursor = context.getContentResolver().query(selectedUri, columns, null, null, null);
        if (cursor == null){
            contentPath = selectedUri.getPath();
        }else{
            cursor.moveToFirst();

            int pathColumnIndex = cursor.getColumnIndex(columns[0]);
            int mimeTypeColumnIndex = cursor.getColumnIndex(columns[1]);

            contentPath = cursor.getString(pathColumnIndex);
            cursor.close();
        }

        return contentPath;

    }

    public static boolean isQQMediaDocument(Uri uri) {
        return "com.tencent.mtt.fileprovider".equals(uri.getAuthority());
    }

    public static Bitmap getImage(String path) {
        try {
            Bitmap src = BitmapFactory.decodeFile(path);

            ExifInterface exif = new ExifInterface(path);
            int rotation = exif.getAttributeInt(ExifInterface.TAG_ORIENTATION, ExifInterface
                    .ORIENTATION_NORMAL);
            Matrix matrix = new Matrix();


            Log.d("GetImage Rotation", "" + rotation);
            switch (rotation) {
                case ExifInterface.ORIENTATION_NORMAL:
                    break;
                case ExifInterface.ORIENTATION_FLIP_HORIZONTAL:
                    matrix.setScale(-1, 1);
                    break;
                case ExifInterface.ORIENTATION_ROTATE_180:
                    matrix.setRotate(180);
                    break;
                case ExifInterface.ORIENTATION_FLIP_VERTICAL:
                    matrix.setRotate(180);
                    matrix.postScale(-1, 1);
                    break;
                case ExifInterface.ORIENTATION_TRANSPOSE:
                    matrix.setRotate(90);
                    matrix.postScale(-1, 1);
                    break;
                case ExifInterface.ORIENTATION_ROTATE_90:
                    matrix.setRotate(90);
                    break;
                case ExifInterface.ORIENTATION_TRANSVERSE:
                    matrix.setRotate(-90);
                    matrix.postScale(-1, 1);
                    break;
                case ExifInterface.ORIENTATION_ROTATE_270:
                    matrix.setRotate(-90);
                    break;
                default:
                    break;
            }

//
            int hight = src.getHeight() > src.getWidth() ? src
                    .getHeight() : src.getWidth();
//
            float scale = 1080.0f / hight;
//
//
            if (scale < 1) {
                matrix.postScale(scale, scale);

            }
            Bitmap dst = Bitmap.createBitmap(src, 0, 0, src.getWidth(), src.getHeight(), matrix, true);
            //src.recycle();
            return dst;
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }

    public static Bitmap getImage(Context context, int mipmapId) {
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inScaled = false;
        Bitmap src = BitmapFactory.decodeResource(context.getResources(), mipmapId, options);
        Matrix matrix = new Matrix();
        int hight = src.getHeight() > src.getWidth() ? src
                .getHeight() : src.getWidth();
        float scale = 1080.0f / hight;
        if (scale < 1) {
            matrix.postScale(scale, scale);
        }
        Bitmap dst = Bitmap.createBitmap(src, 0, 0, src.getWidth(), src.getHeight(), matrix, true);
        return dst;
    }

    public static Bitmap getImage(Context context, int mipmapId, int width, int height) {
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inScaled = false;
        Bitmap src = BitmapFactory.decodeResource(context.getResources(), mipmapId, options);
        Matrix matrix = new Matrix();
        float sx = src.getWidth() * 1.0f/ width;
        float sy = src.getHeight() * 1.0f/ height;
//        float scale = 1080.0f / hight;
//        if (scale < 1) {
            matrix.postScale(sx, sy);
//        }
        Bitmap dst = Bitmap.createBitmap(src, 0, 0, src.getWidth(), src.getHeight(), matrix, true);
        return dst;
    }

    public static Bitmap resizeBitmap(Bitmap src, float sx, int sy) {
        Matrix matrix = new Matrix();
        matrix.postScale(sx, sy);
        Bitmap dst = Bitmap.createBitmap(src, 0, 0, src.getWidth(), src.getHeight(), matrix, true);
        return dst;
    }

    public static void updateAlbum(Context context, File file) {
        Intent intent = new Intent(Intent.ACTION_MEDIA_SCANNER_SCAN_FILE);
        Uri uri = Uri.fromFile(file);
        intent.setData(uri);
        context.sendBroadcast(intent);
    }

    public static PowerManager.WakeLock wakeLock = null;

    public static void acquireWakeLock(Context context) {
        if (wakeLock == null) {
            PowerManager powerManager = (PowerManager) (context.getSystemService(Context.POWER_SERVICE));
            wakeLock = powerManager.newWakeLock(PowerManager.SCREEN_DIM_WAKE_LOCK, "My Tag");
            wakeLock.acquire();
        }
    }

    public static void releaseWakeLock() {
        if (wakeLock != null && wakeLock.isHeld()) {
            wakeLock.release();
            wakeLock = null;
        }
    }
}