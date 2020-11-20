package com.example.lb.lbrocker;

public class webbase {

   public static String post(String data, String key) {
       key+="=";
        int p_s = data.indexOf(key);
        if (p_s == -1) {
            return "";
        }
        int p_e = data.indexOf("&", p_s + 2);
        if (p_e == -1) {
            p_e = data.length();
        }
        p_s += key.length();
        return data.substring(p_s, p_e);
    }

    public static int post_i(String data, String key) {
        key += "=";
        int p_s = data.indexOf(key);
        if (p_s == -1) {
            return 0;
        }
        int p_e = data.indexOf("&", p_s + 2);
        if (p_e == -1) {
            p_e = data.length();
        }
        p_s += key.length();
        return Integer.valueOf(data.substring(p_s, p_e));
    }
}
