package lin.abcdq.vd.camera.util;

import android.content.Context;
import android.content.res.AssetManager;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileOutputStream;

public class AssetUtils {

    public static String asset2cache(Context context, String name) {
        try {
            File folder = new File(context.getObbDir().getAbsolutePath(), "asset");
            if (!folder.exists()) {
                if (!folder.mkdirs()) {
                    return "";
                }
            }
            File file = new File(folder, name);
            if (!file.exists()) {
                if (!file.createNewFile()) {
                    return "";
                }
            }
            AssetManager assetManager = context.getAssets();
            BufferedInputStream bis = new BufferedInputStream(assetManager.open(name));
            FileOutputStream fos = new FileOutputStream(file);
            byte[] bytes = new byte[1024];
            while (bis.read(bytes) > 0) {
                fos.write(bytes);
            }
            return file.getAbsolutePath();
        } catch (Exception e) {
            e.printStackTrace();
        }
        return "";
    }
}
