import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.URL;
import java.util.List;
import com.google.gson.Gson;


public class Intersectionn 
{
    /**
     * @param args
     * @throws Exception 
     */
	
    static class Response
    {
        String lat;
        String lng;
    }
    
    
    public static void main(String[] args) throws Exception 
    {
        Gson gson = new Gson();

        String json = readUrl("http://api.geonames.org/findNearestIntersectionJSON?lat=40.454&lng=-79.921&username=mike");

        Response response = gson.fromJson(json, Response.class);

        System.out.println(response.lat);
        System.out.println(response.lng);

    }

    private static String readUrl(String urlString) throws Exception 
    {
        BufferedReader reader = null;
        try 
        {
            URL url = new URL(urlString);
            reader = new BufferedReader(new InputStreamReader(url.openStream()));
            StringBuffer buffer = new StringBuffer();
            int read;
            char[] chars = new char[1024];
            while ((read = reader.read(chars)) != -1)
                buffer.append(chars, 0, read); 
            
            String reply = buffer.toString();
            String intersection = reply.substring(32, reply.length() - 1);
            //System.out.println(location); //debug
            //System.out.println(s); //debug
            return intersection;
        } 
        finally 
        {
            if (reader != null)
                reader.close();
        }
      
    }
    
    
}