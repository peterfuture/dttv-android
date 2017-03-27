package dttv.app.utils;

import android.annotation.SuppressLint;
import android.text.TextUtils;

import java.text.DecimalFormat;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.Locale;
import java.util.TimeZone;

/**
 * 
 * @author wwt
 * 
 */
public class DateUtils {

	private static DateUtils dateUtils = null;

	public static String YYYYMMDDHHMMSSZZ = "yyyy-MM-dd'T'HH:mm:ss.000Z";
	public static String YYYYMMDDHHMMSSSSS = "yyyy-MM-dd HH:mm:ss.SSS";
	public static String YYYYMMDDHHMMSS = "yyyy-MM-dd HH:mm:ss";
	public static String YYYYMMDDHHMM = "yyyy-MM-dd HH:mm";
	public static String YYYYMMDDHHMM_C = "yyyy年MM月dd日 HH:mm";
	public static String YYYYMMDDHHMM_S = "yyyy/MM/dd HH:mm";
	public static String YYYYMMDDHH = "yyyy-MM-dd HH";
	public static String YYYYMMDD = "yyyy-MM-dd";
	public static String YYYYMMDD_C = "yyyy年MM月dd日";
	public static String YYYYMMDD_S = "yyyy/MM/dd";
	public static String YYYYMM = "yyyy-MM";
	public static String YYYY = "yyyy";
    public static String HHMMSS = "HH:mm:ss";
	public static String HHMMSS_C = "HH小时mm分ss秒";
    public static String MMDDHHMM = "MM-dd HH:mm";

	public DateUtils() {

	}

	public static DateUtils getInstanse() {
		if (dateUtils == null) {
			dateUtils = new DateUtils();
		}
		return dateUtils;
	}

	/*
	 * 十 分钟
	 */
	public int[] getDayHourMin(int time) {// 秒
		int[] tt = new int[] { 0, 0, 0, 0, 0, 0 };
		int min = time / 60;// 0:00分
		int hour = time / 60 / 60;// 0:00:00
		int day = time / 60 / 60 / 24;
		if (day > 0) {
			return tt;
		} else if (hour > 0) {
			tt[0] = hour / 10;
			tt[1] = hour % 10;
			tt[2] = (time - hour * 60 * 60) / 60 / 10;
			tt[3] = (time - hour * 60 * 60) / 60 % 10;
			tt[4] = (time - hour * 60 * 60 - (time - hour * 60 * 60) / 60) / 10;
			tt[5] = (time - hour * 60 * 60 - (time - hour * 60 * 60) / 60) % 10;
		} else if (min > 0) {
			tt[2] = min / 10;
			tt[3] = min % 10;
			tt[4] = (time - min * 60) / 10;
			tt[5] = (time - min * 60) % 10;
		} else {
			tt[4] = time / 10;
			tt[5] = time % 10;
		}
		return tt;
	}

	public String[] getDayHourMin_n(int time) {// 秒
		String[] tt = new String[] { "0", "0", "0", "0", "0", "0" };
		int min = time / 60;// 0:00分
		int hour = time / 60 / 60;// 0:00:00
		int day = time / 60 / 60 / 24;
		if (day > 0) {
			return tt;
		} else if (hour > 0) {
			tt[0] = hour / 10 + "";
			tt[1] = hour % 10 + "";
			tt[2] = (time - hour * 60 * 60) / 60 / 10 + "";
			tt[3] = (time - hour * 60 * 60) / 60 % 10 + "";
			tt[4] = (time - hour * 60 * 60 - (time - hour * 60 * 60) / 60) / 10
					+ "";
			tt[5] = (time - hour * 60 * 60 - (time - hour * 60 * 60) / 60) % 10
					+ "";
		} else if (min > 0) {
			tt[2] = min / 10 + "";
			tt[3] = min % 10 + "";
			tt[4] = (time - min * 60) / 10 + "";
			tt[5] = (time - min * 60) % 10 + "";
		} else {
			tt[4] = time / 10 + "";
			tt[5] = time % 10 + "";
		}
		return tt;
	}

	/**
	 * 时间戳转换成HH:mm:ss
	 * 
	 * @param durationSeconds
	 * @return
	 */
	public static String getDuration(int durationSeconds) {
		int hours = durationSeconds / (60 * 60);
		int leftSeconds = durationSeconds % (60 * 60);
		int minutes = leftSeconds / 60;
		int seconds = leftSeconds % 60;

		StringBuffer sBuffer = new StringBuffer();
		sBuffer.append(addZeroPrefix(hours));
		sBuffer.append(":");
		sBuffer.append(addZeroPrefix(minutes));
		sBuffer.append(":");
		sBuffer.append(addZeroPrefix(seconds));

		return sBuffer.toString();
	}

	public static String addZeroPrefix(int number) {
		if (number < 10) {
			return "0" + number;
		} else {
			return "" + number;
		}
	}

	// 获得今天星期
	public int getToadayWeek() {
		String week = "";
		Calendar c = Calendar.getInstance();
		c.setTime(new Date(System.currentTimeMillis()));
		int dayOfWeek = c.get(Calendar.DAY_OF_WEEK);
		switch (dayOfWeek) {
		case 1:
			week = "星期日";
			break;
		case 2:
			week = "星期一";
			break;
		case 3:
			week = "星期二";
			break;
		case 4:
			week = "星期三";
			break;
		case 5:
			week = "星期四";
			break;
		case 6:
			week = "星期五";
			break;
		case 7:
			week = "星期六";
			break;
		}
		return dayOfWeek - 1;
	}

	// 获取当前系统时间
	@SuppressLint("SimpleDateFormat")
	public String getNowDate(String DateType) {
		SimpleDateFormat df = new SimpleDateFormat(DateType);// 设置日期格式
		String date = df.format(new Date());// System.currentTimeMillis()
//		System.out.println(date);// new Date()为获取当前系统时间
		return date;
	}

	// 获得当前系统的时间戳 毫秒
	public long getNowTime() {
		// 方法 一
		long time = System.currentTimeMillis();
		// 方法 二
		// long time=Calendar.getInstance().getTimeInMillis();
		// 方法 三
		// long time=new Date().getTime();
		return time;// 毫秒
	}

	public long getNowOldTime(String old) {
		SimpleDateFormat df = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
		Date now;
		long l = 0;
		try {
			now = df.parse("2004-03-26 13:31:34");

			Date date = df.parse("2004-01-02 11:30:53");
			l = now.getTime() - date.getTime();
		} catch (ParseException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		return l;
	}

	// 获得当天0点时间戳
	public long getTimesmorning() {
		Calendar cal = Calendar.getInstance();
		cal.set(Calendar.HOUR_OF_DAY, 0);
		cal.set(Calendar.SECOND, 0);
		cal.set(Calendar.MINUTE, 0);
		cal.set(Calendar.MILLISECOND, 0);
		return cal.getTimeInMillis();
	}

	// 获得当天24点时间戳
	public long getTimesnight() {
		Calendar cal = Calendar.getInstance();
		cal.set(Calendar.HOUR_OF_DAY, 24);
		cal.set(Calendar.SECOND, 0);
		cal.set(Calendar.MINUTE, 0);
		cal.set(Calendar.MILLISECOND, 0);
		return cal.getTimeInMillis();
	}

	// 获得本周一0点时间戳
	public long getTimesWeekmorning() {
		Calendar cal = Calendar.getInstance();
		cal.set(cal.get(Calendar.YEAR), cal.get(Calendar.MONDAY),
				cal.get(Calendar.DAY_OF_MONTH), 0, 0, 0);
		cal.set(Calendar.DAY_OF_WEEK, Calendar.MONDAY);
		return cal.getTimeInMillis();
	}

	// 获得本周日24点时间戳
	public long getTimesWeeknight() {
		Calendar cal = Calendar.getInstance();
		cal.set(cal.get(Calendar.YEAR), cal.get(Calendar.MONDAY),
				cal.get(Calendar.DAY_OF_MONTH), 0, 0, 0);
		cal.set(Calendar.DAY_OF_WEEK, Calendar.MONDAY);
		return (cal.getTime().getTime() + (7 * 24 * 60 * 60 * 1000));
	}

	// 获得本月第一天0点时间戳
	public long getTimesMonthmorning() {
		Calendar cal = Calendar.getInstance();
		cal.set(cal.get(Calendar.YEAR), cal.get(Calendar.MONDAY),
				cal.get(Calendar.DAY_OF_MONTH), 0, 0, 0);
		cal.set(Calendar.DAY_OF_MONTH,
				cal.getActualMinimum(Calendar.DAY_OF_MONTH));
		return cal.getTimeInMillis();
	}

	// 获得本月最后一天24点时间戳
	public long getTimesMonthnight() {
		Calendar cal = Calendar.getInstance();
		cal.set(cal.get(Calendar.YEAR), cal.get(Calendar.MONDAY),
				cal.get(Calendar.DAY_OF_MONTH), 0, 0, 0);
		cal.set(Calendar.DAY_OF_MONTH,
				cal.getActualMaximum(Calendar.DAY_OF_MONTH));
		cal.set(Calendar.HOUR_OF_DAY, 24);
		return cal.getTimeInMillis();
	}

	/**
	 * 两时间相减
	 * 
	 * @param time
	 * @param time2
	 * @return
	 */
	public long TwoTimeSub(String nowtime, String othertime, String type) {
		SimpleDateFormat df = new SimpleDateFormat(type);// "yyyy-MM-dd HH:mm:ss"
		Date now;
		Date date;
		long sub = 0;
		try {
			now = df.parse(nowtime);
			date = df.parse(othertime);
			sub = now.getTime() - date.getTime();
		} catch (ParseException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		return sub;
	}

	/**
	 * 判断2时间谁大谁小
	 * 
	 * @param nowtime
	 * @param othertime
	 * @return
	 */
	public boolean JudgeTwoTime(String nowtime, String othertime, String type) {
		SimpleDateFormat df = new SimpleDateFormat(type);// "yyyy-MM-dd HH:mm:ss"
		Date now;
		Date date;
		boolean flag = false;
		try {
			now = df.parse(nowtime);
			date = df.parse(othertime);
			if (now.getTime() - date.getTime() > 0) {
				flag = true;
			}
		} catch (ParseException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		return flag;
	}

    /**
     * 把格式 "2014-10-14T14:48:03.533Z" 转换成  "2014-10-14 14:48:03"
     * @param date
     * @return
     */
	public String changeDate(String date){
		if (TextUtils.isEmpty(date)) {
			return null;
		}
		String d=date.replaceFirst("T"," ");
		String dd=null;
		if (d.indexOf(".")>0) {
			dd=d.substring(0, d.indexOf("."));
		}
		return dd;
	}
    /*
     * "yyyy-MM-dd HH:mm:ss"转换成"MM-dd HH:mm"
     * type转换成typeto
     */
    public String typeTotype(String text, String type, String typeto) {
        if (TextUtils.isEmpty(text))return "";
        SimpleDateFormat s1 = new SimpleDateFormat(type);
        SimpleDateFormat s2 = new SimpleDateFormat(typeto);
        Date d1;
        String datee = null;
        try {
            d1=s1.parse(text);
            datee=s2.format(d1);
//            System.out.println(""+s2.format(d1));
        } catch (ParseException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        return datee;
    }
	/**
	 * 时间戳转换成(时分秒s) day hour min ss
	 * 
	 * @param l
	 * @return
	 */
	public String getDayHourMinSS(long l) {
		long day = l / (24 * 60 * 60 * 1000);
		long hour = (l / (60 * 60 * 1000) - day * 24);
		long min = ((l / (60 * 1000)) - day * 24 * 60 - hour * 60);
		long s = (l / 1000 - day * 24 * 60 * 60 - hour * 60 * 60 - min * 60);
//		System.out.println("" + day + "天" + hour + "小时" + min + "分" + s + "秒");
		return day + " " + hour + ":" + min + ":" + s;
	}
	/**
	 * 时间戳转换成(时分秒s) day hour min ss
	 *
	 * @param ms
	 * @return
	 */
	public static String getmstodate(long ms, String dataType) {//毫秒数
		//初始化Formatter的转换格式
		if (ms>0){//&&ms- TimeZone.getDefault().getRawOffset()>0
			SimpleDateFormat formatter = new SimpleDateFormat(dataType, Locale.CHINA);//"HH:mm:ss"
//			String hms = formatter.format(1000*ms);//why multiply 1000
			String hms = formatter.format(ms);//why multiply 1000
//			String hms = formatter.format(ms- TimeZone.getDefault().getRawOffset());//减8小时时差

//		System.out.println(hms);
			return hms;
		}else return "00:00:00";
	}
	public static String getmstodated(long ms, String dataType) {//毫秒数
		//初始化Formatter的转换格式
		if (ms>0&&ms- TimeZone.getDefault().getRawOffset()>0){//
			SimpleDateFormat formatter = new SimpleDateFormat(dataType, Locale.CHINA);//"HH:mm:ss"
//			String hms = formatter.format(ms- TimeZone.getDefault().getRawOffset());//减8小时时差
			String hms = formatter.format(ms);//why multiply 1000
			return hms;
		}else return "00:00:00";
	}

    /**
     * DecimalFormat
     * 取double 的整数
     */
    public String getInt(double date) {
        DecimalFormat df = new DecimalFormat("#");
        return df.format(date);
    }

	/**
	 * DecimalFormat转换最简便 保留1位小数
	 */
	public String getOne(double date) {
		DecimalFormat df = new DecimalFormat("#.0");
		return df.format(date);
	}

//	/**
//	 * DecimalFormat转换最简便 保留2位小数
//	 */
//	public String getDouble(double date) {
//		String str = new Double(date).toString();
//		if (str.contains(".")) {
//			str = str.substring(0, str.indexOf(".") + 3);
//		} else {
//			str = str + ".00";
//		}
//		return str;
////		return String.format("%.2f", date);
//	}

	public String getDouble(double doubleValue) {
		// 保留4位小数
		java.text.DecimalFormat df = new java.text.DecimalFormat("#.0000");
		String result = df.format(doubleValue);

		// 截取第一位
		String index = result.substring(0, 1);

		if (".".equals(index)) {
			result = "0" + result;
		}

		// 获取小数 . 号第一次出现的位置
		int inde = firstIndexOf(result, ".");

		// 字符串截断
		return result.substring(0, inde + 3);
	}

	public static int firstIndexOf(String str, String pattern) {
		for (int i = 0; i < (str.length() - pattern.length()); i++) {
			int j = 0;
			while (j < pattern.length()) {
				if (str.charAt(i + j) != pattern.charAt(j))
					break;
				j++;
			}
			if (j == pattern.length())
				return i;
		}
		return -1;
	}


	/**
	 * DecimalFormat转换最简便 保留3位小数
	 */
	public String getThree(double date) {
		DecimalFormat df = new DecimalFormat("#.000");
		return df.format(date);
	}

	/**
	 *
	 * @param date
	 * @return
	 */
	public String getDataType(long date) {
		if (0<date&&date<10000){
			return date+"元";
		}else if(date>=10000&&date<1000000){
			return getInt(date/10000)+"万";
//		}
//		else if (date>=1000000&&date<10000000){
//			return getInt(date/1000000)+"百万";
//		}else if (date>=10000000&&date<100000000){
//			return getInt(date/10000000)+"千万";
//
		}else return getDouble(date/100000000)+"亿";

	}

    //
    public float changeDate(float date){
        float dd=0;
        if (0<=date&&date<0.5)dd=0.0f;
        else if (0.5<=date&&date<1.0)dd=0.5f;
        else if (1.0<=date&&date<1.5)dd=1.0f;
        else if (1.5<=date&&date<2.0)dd=1.5f;
        else if (2.0<=date&&date<2.5)dd=2.0f;
        else if (2.5<=date&&date<3.0)dd=2.5f;
        else if (3.0<=date&&date<3.5)dd=3.0f;
        else if (3.5<=date&&date<4.0)dd=3.5f;
        else if (4.0<=date&&date<4.5)dd=4.0f;
        else if (4.5<=date&&date<5.0)dd=4.5f;
        else if (5.0<=date&&date<5.5)dd=5.0f;
        return  dd;
    }

	public static String[] getCountDownHMS(long countDownTime) {
		String[] hms = new String[3];
		int h = (int) countDownTime / (60 * 60);
		int m = (int) (countDownTime % (60 * 60) / 60);
		int s = (int) (countDownTime % 60);

		hms[0] = h > 9 ? "" + h : "0" + h;
		hms[1] = m > 9 ? "" + m : "0" + m;
		hms[2] = s > 9 ? "" + s : "0" + s;

		return hms;
	}

}
