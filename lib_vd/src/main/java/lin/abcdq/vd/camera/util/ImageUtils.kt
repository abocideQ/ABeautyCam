package lin.abcdq.ffmpeg.util

import android.graphics.ImageFormat
import android.graphics.Rect
import android.graphics.YuvImage
import android.media.Image
import android.os.Build
import androidx.annotation.RequiresApi
import java.io.ByteArrayOutputStream
import java.nio.ByteBuffer

@RequiresApi(Build.VERSION_CODES.LOLLIPOP)
internal class ImageUtils {

    companion object {
        fun image2YUV420888(image: Image): ByteArray {
            val imageWidth = image.width
            val imageHeight = image.height
            val planes = image.planes
            val data = ByteArray(
                imageWidth * imageHeight *
                        ImageFormat.getBitsPerPixel(ImageFormat.YUV_420_888) / 8
            )
            var offset = 0
            for (plane in planes.indices) {
                val buffer = planes[plane].buffer
                val rowStride = planes[plane].rowStride
                // Experimentally, U and V planes have |pixelStride| = 2, which
                // essentially means they are packed.
                val pixelStride = planes[plane].pixelStride
                val planeWidth = if ((plane == 0)) imageWidth else imageWidth / 2
                val planeHeight = if ((plane == 0)) imageHeight else imageHeight / 2
                if (pixelStride == 1 && rowStride == planeWidth) {
                    // Copy whole plane from buffer into |data| at once.
                    buffer[data, offset, planeWidth * planeHeight]
                    offset += planeWidth * planeHeight
                } else {
                    // Copy pixels one by one respecting pixelStride and rowStride.
                    val rowData = ByteArray(rowStride)
                    for (row in 0 until planeHeight - 1) {
                        buffer[rowData, 0, rowStride]
                        for (col in 0 until planeWidth) {
                            data[offset++] = rowData[col * pixelStride]
                        }
                    }
                    // Last row is special in some devices and may not contain the full
                    // |rowStride| bytes of data.
                    // See http://developer.android.com/reference/android/media/Image.Plane.html#getBuffer()
                    buffer[rowData, 0, Math.min(rowStride, buffer.remaining())]
                    for (col in 0 until planeWidth) {
                        data[offset++] = rowData[col * pixelStride]
                    }
                }
            }
            return data
        }

        fun image2PixelRgba(image: Image): ByteArray {
            return image.planes[0].buffer.array()
        }

        fun image2NV21(image: Image): ByteArray {
            val nv21: ByteArray
            val yBuffer: ByteBuffer = image.planes[0].buffer
            val vuBuffer: ByteBuffer = image.planes[2].buffer
            val ySize: Int = yBuffer.remaining()
            val vuSize: Int = vuBuffer.remaining()
            nv21 = ByteArray(ySize + vuSize)
            yBuffer.get(nv21, 0, ySize)
            vuBuffer.get(nv21, ySize, vuSize)
            return nv21
        }

        fun image2JPEG(image: Image): ByteArray {
            val data: ByteArray
            when (image.format) {
                ImageFormat.JPEG -> {
                    val planes = image.planes
                    val buffer: ByteBuffer = planes[0].buffer
                    data = ByteArray(buffer.capacity())
                    buffer.get(data)
                    return data
                }
                ImageFormat.YUV_420_888 -> {
                    data = NV21toJPEG(
                        YUV_420_888toNV21(image),
                        image.width, image.height
                    )
                }
                else -> {
                    data = ByteArray(1)
                }
            }
            return data
        }

        private fun YUV_420_888toNV21(image: Image): ByteArray {
            val nv21: ByteArray
            val yBuffer: ByteBuffer = image.planes[0].buffer
            val vuBuffer: ByteBuffer = image.planes[2].buffer
            val ySize: Int = yBuffer.remaining()
            val vuSize: Int = vuBuffer.remaining()
            nv21 = ByteArray(ySize + vuSize)
            yBuffer.get(nv21, 0, ySize)
            vuBuffer.get(nv21, ySize, vuSize)
            return nv21
        }

        private fun NV21toJPEG(nv21: ByteArray, width: Int, height: Int): ByteArray {
            val out = ByteArrayOutputStream()
            val yuv = YuvImage(nv21, ImageFormat.NV21, width, height, null)
            yuv.compressToJpeg(Rect(0, 0, width, height), 100, out)
            return out.toByteArray()
        }
    }
}