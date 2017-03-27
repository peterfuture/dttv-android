package dttv.app.utils;

import android.os.Environment;
import android.util.Log;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.text.DecimalFormat;

/**
 *
 */
public class FileNewUtils {

    public static String TAG=FileNewUtils.class.getSimpleName();

	private static FileNewUtils instance;

	public FileNewUtils() {

	}

	public static FileNewUtils getInstance() {
		if (instance == null) {
			instance = new FileNewUtils();
		}
		return instance;
	}

	public long getFileSizes(File f) throws Exception {
		long s = 0;
		if (f.exists()) {
			FileInputStream fis = null;
			fis = new FileInputStream(f);
			s = fis.available();
		} else {
			f.createNewFile();
			System.out.println("文件不存在");
		}
		return s;
	}

	public long getFileSize(File f) throws Exception {
		long size = 0;
		File flist[] = f.listFiles();
		for (int i = 0; i < flist.length; i++) {
			if (flist[i].isDirectory()) {
				size = size + getFileSize(flist[i]);
			} else {
				size = size + flist[i].length();
			}
		}
		return size;
	}

	public String FormetFileSize(long fileS) {// 转换文件大小
		DecimalFormat df = new DecimalFormat("#.00");
		String fileSizeString = "";
		if (fileS == 0) {
			fileSizeString = "0.00B";
		} else if (fileS < 1024) {
			fileSizeString = df.format((double) fileS) + "B";
		} else if (fileS < 1048576) {
			fileSizeString = df.format((double) fileS / 1024) + "K";
		} else if (fileS < 1073741824) {
			fileSizeString = df.format((double) fileS / 1048576) + "M";
		} else {
			fileSizeString = df.format((double) fileS / 1073741824) + "G";
		}
		return fileSizeString;
	}

	public long getlist(File f) {// 递归求取目录文件个数
		long size = 0;
		File flist[] = f.listFiles();
		size = flist.length;
		for (int i = 0; i < flist.length; i++) {
			if (flist[i].isDirectory()) {
				size = size + getlist(flist[i]);
				size--;
			}
		}
		return size;
	}

	public String getFileSize() {
		long size = 0;
		try {
			size = getFileSize(new File(
					Environment.getExternalStorageDirectory(),
					"LazyList"));
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		return FormetFileSize(size);
	}



	// 读取文本文件中的内容
	public String ReadTxtFile(String strFilePath) {
		String path = strFilePath;
		StringBuffer content = new StringBuffer(); // 文件内容字符串
		// 打开文件
		File file = new File(path);
		// 如果path是传递过来的参数，可以做一个非目录的判断
		if (file.isDirectory()) {
			Log.d("TestFile", "The File doesn't not exist.");
		} else {
			try {
				InputStream instream = new FileInputStream(file);
				if (instream != null) {
					InputStreamReader inputreader = new InputStreamReader(
							instream);
					BufferedReader buffreader = new BufferedReader(inputreader);
					String line;
					// 分行读取
					while ((line = buffreader.readLine()) != null) {
						content.append(line + "\n");
					}
					instream.close();
				}
			} catch (java.io.FileNotFoundException e) {
				Log.d("TestFile", "The File doesn't not exist.");
			} catch (IOException e) {
				Log.d("TestFile", e.getMessage());
			}
		}
		return content.toString();
	}

	/**
	 * 判断是否有sd卡
	 * 
	 * @return
	 */
	public static String getSdPath() {
		File sdDir = null;
		boolean sdCardExist = Environment.getExternalStorageState().equals(
				Environment.MEDIA_MOUNTED); // 判断sd卡是否存在
		if (sdCardExist) {
			sdDir = Environment.getExternalStorageDirectory();// 获取跟目录
			return sdDir.toString();
		} else {
			return null;
		}
	}



    /**
     * 新建一个文件
     * @param path :路径
     * @param filename  : 文件名
     * @return
     */
    public File makeFile(String path, String filename) {
        // 判断SDcard是否存在
        boolean sdCardExist = Environment.getExternalStorageState().equals(
                Environment.MEDIA_MOUNTED); // 判断sd卡是否存在
        if (!sdCardExist) {
            Log.d(TAG,"sd卡不存在");
            return null;
        }
        File dir = new File(path);
        // 路径是否存在，目录是否存在
        if (!dir.exists() && !dir.isDirectory()) {// 路径不存在就创建路径
            Log.d(TAG,"sd目录路径创建");
            // 可以创建多级目录，无论是否存在父目录
            dir.mkdirs();// 创建文件夹
        }
        File f = new File(path, filename);
        try {
            if (!f.exists()) {
                Log.d(TAG,"sd目录路径文件创建");
                f.createNewFile();// 创建文件
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
        return f;
    }


    /**
     * FileOutputStream :写入文件
     * 文件输出流是一种用于处理原始二进制数据的字节流类。为了将数据写入到文件中，必须将数据转换为字节，并保存到文件
     *
     * @param path
     * @param filename
     * @param text
     */
    public void writeFileToSdcardByFileOutputStream(String path,
                                                    String filename, String text) {
        File f = makeFile(path, filename);// 文件
        if (f != null) {// SDcard不存在
            try {
                FileOutputStream fop = new FileOutputStream(f);
                byte[] bytes = text.getBytes("UTF-8");// GBK UTF-8
                fop.write(bytes);
                fop.flush();
                fop.close();
                Log.d(TAG,"文件写入成功");
            } catch (IOException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }

	public void writeFileToSdcardByFileOutputStream(String path,
                                                    String filename, byte[] bytes) {
		File f = makeFile(path, filename);// 文件
		if (f != null) {// SDcard不存在
			try {
				FileOutputStream fop = new FileOutputStream(f);
//				byte[] bytes = text.getBytes("UTF-8");// GBK UTF-8
				fop.write(bytes);
				fop.flush();
				fop.close();
				Log.d(TAG,"文件写入成功");
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
	}

    /**
     * FileInputStream :读取文件
     *
     * @param path
     * @param filename
     * @return
     */
    public String readFileToSdcardByFileInputStream(String path,
                                                    String filename) {
        String result = null;
        File f = makeFile(path, filename);// 文件
        StringBuilder text = new StringBuilder();
        if (f != null) {// SDcard不存在
            try {
                FileInputStream fin=new FileInputStream(f);
                if (fin!=null) {
                    InputStreamReader inputreader=new InputStreamReader(fin);
                    BufferedReader bufferedReader=new BufferedReader(inputreader);
                    // 分行读取
                    while ((result = bufferedReader.readLine()) != null) {
                        text.append(result);
                        text.append('\n');
                    }
                    inputreader.close();
                    fin.close();
                    Log.d(TAG,"文件读取成功");
                }
            }catch (java.io.FileNotFoundException e) {
                Log.d("TestFile", "The File doesn't not exist.");
            } catch (IOException e) {
                Log.d("TestFile", e.getMessage());
            }
        }
        return text.toString();
    }


    /**
     * 删除目录下的所有文件
     *
     * @param filepath
     * @throws java.io.IOException
     */
    public void del(String filepath){
        try {
        File f = new File(filepath);// 定义文件路径
        if (f.exists() && f.isDirectory()) {// 判断是文件还是目录
            if (f.listFiles().length == 0) {// 若目录下没有文件则直接删除
                f.delete();
            } else {// 若有则把文件放进数组，并判断是否有下级目录
                File delFile[] = f.listFiles();
                int i = f.listFiles().length;
                for (int j = 0; j < i; j++) {
                    if (delFile[j].isDirectory()) {
                        del(delFile[j].getAbsolutePath());// 递归调用del方法并取得子目录路径
                    }
                    delFile[j].delete();// 删除文件
                }
            }
        }
        }catch (Exception e){

        }
    }

    /**
     * 判断文件是否存在
     * @param path
     * @param filename
     * @return
     */
    public static boolean isFileNameExist(String path, String filename) {
        // 判断SDcard是否存在
        boolean sdCardExist = Environment.getExternalStorageState().equals(
                Environment.MEDIA_MOUNTED); // 判断sd卡是否存在
        if (!sdCardExist) {
            Log.i(TAG,"sd卡不存在");
            return false;
        }
        File dir = new File(path);
        // 路径是否存在，目录是否存在
        if (!dir.exists() && !dir.isDirectory()) {// 路径不存在
            return false;
        }
        File f = new File(path, filename);
        if (!f.exists()) {
            return false;
        }else return true;
    }
}
