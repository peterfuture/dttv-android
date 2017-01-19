package app.dttv.dttvlib.utils;

public class TimesUtil {

    public static String getTime(int i) {
        String result = "";
        int minute = i / 60;
        int hour = minute / 60;
        int second = i % 60;
        minute %= 60;
        result = String.format("%02d:%02d:%02d", hour, minute, second);
        return result;
    }
}
