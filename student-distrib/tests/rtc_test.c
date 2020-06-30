#include "../drivers/rtc.h"
#include "../lib/lib.h"
#include "../drivers/fs.h"
#include "tests_files.h"

/*
 * test_rtc
 *   DESCRIPTION: Test the RTC driver with two sample prices of code that execute all of the functions 
 *                associated with the driver.  
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void 
test_rtc()
{
    int i = 0;
    int j = 0;
    int rate = 4;
    int mr_flag = 0;
    rtc_open();
    //Loop through and print an increasing integer with increasing rate 
    while(mr_flag < 5)
    {
        if(j <= 0)
        {
            printf("\n************************************* %d Hz *************************************\n", rate);
        }
        //waits for the the next interrupt from the rtc  
        rtc_write(NULL,&rate,NULL);
        for (j = 0; j < 30; j++)
        {
            rtc_read(NULL, NULL, NULL);
            printf("%d  ", i++);
        }

        if(j >= 30)
        {
            //increases the rtc rate 
            rate = rate << 1;
            rtc_write(NULL,&rate,NULL);
            if (rate >=10){
                printf("\n************************************ %d Hz *************************************\n", rate);
            }else{
                printf("\n************************************* %d Hz *************************************\n", rate);
            }
            i = 0;
            mr_flag++;
        }
    }
    //opens the fish file 
    printf("FISH TIME\n");
    dentry_t frame0, frame1;
    dentry_t *curr, *other, *temp;
    char buf[174];

    read_dentry_by_name("frame0.txt", &frame0);
    read_dentry_by_name("frame1.txt", &frame1);

    curr = &frame0;
    other = &frame1;


    //prints the fish and increases the the rate of print 
    //by increasing the etc rate and then decreases the rtc rate 
    mr_flag = 0;
    i = 0;
    rate = 2;
    int down_flag = 0;
    rtc_write(NULL, &rate, NULL);
    while (mr_flag++ < 100)
    {
        //increase the RTC rate 
        if(i >= 4 && down_flag == 0)
        {
            i = 0;
            rate = rate << 1;
            if(rate >= 1024)
            {
                down_flag = 1;
            }
        }
        //decrease the RTC rate 
        if(i >= 4 && down_flag == 1)
        {
            i = 0;
            rate = rate >> 1;
            if(rate <= 2)
                down_flag = 0;
        }
        rtc_write(NULL, &rate, NULL);
        rtc_read(NULL, NULL, NULL);
        clear();
        // read_data((*curr).inode_index, 0, (char *)buf, 200);
        temp = curr;
        curr = other;
        other = temp;
        printf("%s", buf);
        printf("MP1 NEVER FORGET - %d Hz\n", rate);
        i++;
    }
    clear();   
}

