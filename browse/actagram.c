//----------------------------------------------------------------------------------
// HTML based image browser to use with imgcomp output.
//
// Make HTML page for actagram view.
//
// Imgcomp and html browsing tool is licensed under GPL v2 (see README.txt)
//---------------------------------------------------------------------------------- 
#include <stdio.h>
#include <errno.h>
#include <memory.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <sys/statvfs.h>

#include "view.h"


int is_valid_day(int day)
{
    return day >=1 && day <= 31;
}

int is_valid_month(int month)
{
    return month >=1 && month <= 12;
}

int is_valid_year(int year)
{
    return year >=10 && year <= 99;
}

int is_valid_date(int date)
{
    return is_valid_day(date % 100)
        && is_valid_month((date / 100) % 100)
        && is_valid_year((date / 10000) % 100);
}

int read_holiday_config(int holidays[])
{
    FILE *f = fopen("holiday.conf", "r");
    if (f == NULL) {
        fprintf(stderr, "[ERROR]: Could not open file holiday.conf.\n");
        return 0;
    }
    printf("\n[DEBUG]: Read holiday.conf file.");
    printf("<script>\n"
            "console.log(\"config test\")\n"
           "\n</script>\n");

    int date;
    int size = 0;
    while (fscanf(f, "%d", &date) != EOF) {
        if (is_valid_date(date)) {
            holidays[size++] = date;
        } else {
            fprintf(stderr, "[ERROR]: %d is not a valid date.\n", date);
        }
    }
    fclose(f);
    return size;
}


//----------------------------------------------------------------------------------
// Determine if it's a weekend day or a holiday
// Returns < 0 for weekday, >= 0 for weekend/holiday.  3 lsbs indicate day of week.
//----------------------------------------------------------------------------------
int IsWeekendString(char * DirString)
{
    int a, daynum, weekday;
    int y,m,d;
    daynum=0;
    for (a=0;a<6;a++){
        if (DirString[a] < '0' || DirString[a] > '9') break;
        daynum = daynum*10+DirString[a]-'0';
    }
    if (a != 6 || DirString[6] != '\0'){
        // dir name must be six digits only.

        return -10;
    }

    d = daynum%100;
    m = (daynum/100)%100;
    y = daynum/10000 + 2000;
    //printf("%d %d %d",d,m,y);

    {
        // A clever bit of code from the internet!
        static int t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
        y -= m < 3;
        weekday = ( y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7;
        // Sunday = 0, Saturday = 6
    }

    // New brunswick holidays thru end of 2021.
    /*
     *
    static const int Holidays[] = {190701, 190902, 191225,
                200101, 200217, 200410, 200518, 200701, 200907, 201225,
                210101, 210402, 210524, 210701, 210802, 210906, 211227};
    */
    int holidays[200];
    int size = read_holiday_config(holidays);
    for (int j = 0; j < size; j++) {
        printf("\nj: %d, %d\n", j, holidays[j]);
    }

    for (a=0;a<size;a++){
        // Check if date is a holday.
        if (holidays[a] == daynum) return weekday;
    }

    if (weekday == 0 || weekday == 6) return weekday;

    return weekday-8;
}

//----------------------------------------------------------------------------------
// Do actagram output
//----------------------------------------------------------------------------------
void ShowActagram(int all, int h24)
{
    int daynum;
    VarList DayDirs;
    memset(&DayDirs, 0, sizeof(DayDirs));
    int BinsPerHour = 15;
    int MinutesPerBin = 60/BinsPerHour;

    printf("<head\n"
           "<title>Actagram</title>\n"
           "<head><meta charset=\"utf-8\"/>\n"
           "</head>");

    printf(
        "<style type=text/css>\n"
        "  body { font-family: sans-serif; font-size: 20;}\n"
        "  span.wkend {background-color: #E8E8E8}\n"
        "  pre { font-size: 10;}\n"
        "  a {text-decoration: none;}\n"
        "</style>\n");
    
    printf("Actagram: <a href='view.cgi?/'>[Back]</a>"
           "<a href='view.cgi?actagram,all'>[All]</a></big>\n");
    printf("<pre><b>");
    
    CollectDirectory("pix/", NULL, &DayDirs, NULL);

    if (all){
        daynum = 0;
    }else{
        daynum = DayDirs.NumEntries-60;
    }

    if (daynum < 0) daynum = 0;

    int prevwkd = 6, thiswkd=6;
    const int NUMBINS = 15*24;

    // Only show from 7 am to 8 pm
    int from = 7*BinsPerHour;
    int to = BinsPerHour*20+1;


        //printf("n");
    
    int ShowLegend = 1;
    
    int HrefOpen = 0;
    
    for (;;daynum++){
        int bins[NUMBINS];
        char BinImgName[NUMBINS][24];
        char DirName[NUMBINS];
        int a, h;
        VarList HourDirs;
        char * DayName = DayDirs.Entries[daynum].Name;

        if (ShowLegend || daynum == DayDirs.NumEntries){
            // Show legend at the start and end of the actagram view.
            printf("Time:");
            for (int a=from;a<=to;a++){
                char nc = ' ';
                if (a % BinsPerHour == 0 && a+5 < to){
                    printf("%02d:00",a/BinsPerHour);
                    a += 5;
                }
                putchar(nc);
            }
            ShowLegend = 0;
        }
        if (daynum >= DayDirs.NumEntries){
            printf("\n");
            break;
        }
        
        memset(BinImgName, 0, sizeof(BinImgName));
        if (strcmp(DayDirs.Entries[daynum].Name, "saved") == 0) continue;
        
        memset(bins, 0, sizeof(bins));

        printf("<script>\n"
            "console.log(\"testing is weekend\")\n"
           "\n</script>\n");

        int isw = IsWeekendString(DayDirs.Entries[daynum].Name);

        thiswkd = isw & 7;
        if (thiswkd != (prevwkd+1) %7){
            printf("<br>");
        }
        prevwkd = thiswkd;


        if (isw >= 0) printf("<span class=\"wkend\">");
        
        printf("<a href='view.cgi?%s'>%s</a> ",DayName, DayName+2);
        sprintf(DirName, "pix/%s",DayName);
        memset(&HourDirs, 0, sizeof(HourDirs));
        
        CollectDirectory(DirName, NULL, &HourDirs, NULL);
        
        for (h=0;h<HourDirs.NumEntries;h++){
            int a;
            VarList HourPix;
            memset(&HourPix, 0, sizeof(HourPix));
            sprintf(DirName, "pix/%s/%s",DayName, HourDirs.Entries[h].Name);
            CollectDirectory(DirName, &HourPix, NULL, ImageExtensions);
            
            for (a=0;a<HourPix.NumEntries;a++){
                int minute, binno;
                char * Name;
                Name = HourPix.Entries[a].Name;
                
                minute = (Name[5]-'0')*60*10 + (Name[6]-'0')*60
                       + (Name[7]-'0')*10    + (Name[8]-'0');
                       
                binno = minute/MinutesPerBin;
                if (binno >= 0 && binno < NUMBINS){
                    bins[binno] += 1;
                    if (bins[binno] < 10) strncpy(BinImgName[binno],Name,23);
                }
            }
            free(HourPix.Entries);
        }
        free (HourDirs.Entries);
        
        if (h24){
            from=0;
            to=BinsPerHour*24;
        }
        for (a=from;a<=to;a++){
            char nc = ' ';
            if (a % BinsPerHour == 0) nc = ':';
            if (a % (BinsPerHour*6) == 0) nc = '|';
            
            //if (bins[a] >= 1 && nc == ' ') nc = '.';
            if (bins[a] >= 5) nc = '-';
            if (bins[a] >= 12) nc = '1';
            if (bins[a] >= 40) nc = '2';
            if (bins[a] >= 100) nc = '#';
            
            if (bins[a] >= 1){
                printf("<a href='view.cgi?%s/%02d/#%s'",DayName,a/BinsPerHour, BinImgName[a]);
                printf(" onmouseover=\"mmo('%s/%02d/%s')\"",DayName,a/BinsPerHour,BinImgName[a]);
                printf(">%c", nc);
                HrefOpen = 1; // Don't close the href till after the next char, makes it easier to hover over single dot.
            }else{
                putchar(nc);
                if (HrefOpen) printf("</a>");
                HrefOpen = 0;
            }
        }
        if (HrefOpen) printf("</a>");
        HrefOpen = 0;
        if (isw >= 0) printf("</span>");
        printf("\n");
    }
    free(DayDirs.Entries);
    
    
    // Add javascript for hover-over preview when showing a whole day's worth of images
    printf("</pre><small id='prevn'></small><br>\n"
           "<a id='prevh' href=""><img id='preview' src='' width=0 height=0></a>\n");


    // Javascript
    printf("<script>\n" // Script to resize the image to the right aspect ratio
           "function sizeit(){\n"
           "  var h = 300\n"
           "  var w = h/el.naturalHeight*el.naturalWidth;\n"
           "  if (w > 850){ w=850;h=w/el.naturalWidth*el.naturalHeight;}\n"
           "  el.width=w;el.height=h;\n"
           "}\n");
           
    printf("function mmo(str){\n"
           "el = document.getElementById('preview')\n"
           "   el.src = '/pix/'+str\n"
           "   el.onload = sizeit\n"
           "   var eh = document.getElementById('prevh')\n"
           "   eh.href = '/view.cgi?'+str.substring(0,9)+'/#'+str.substring(17,21)\n"
           "   var en = document.getElementById('prevn')\n"
           "   en.innerHTML = str + ' &nbsp; &nbsp; 20'"
           " + str.substring(0, 2)+'-'+str.substring(2,4)+'-'+str.substring(4,6)\n"
           " + ' &nbsp;'+str.substring(15, 17)+':'+str.substring(17,19);"
           "}\n</script>\n");
}




