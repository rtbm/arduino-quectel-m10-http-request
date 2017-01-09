bool isGprsReady = false;
bool isRequestDone = false;

int state = 0;
int reqState = 0;

void setup()
{
    Serial.begin(115200);
    Serial1.begin(115200);
}

String atCmd(String cmd)
{
    String res = "";
    char ch = 0;

    Serial.println("Sent: " + cmd);

    Serial1.print(cmd + '\r');

    while(!Serial1.available()) {}

    while (Serial1.available())
    {
        ch = Serial1.read();
        res += ch;
        delay(5);
    }

    Serial.println("Received: " + res);

    return res;
}

void initModem(String PIN, String APN)
{
    String res = "";

    switch (state)
    {
    case 0:
    {
        res = atCmd("ATE0");  // command echo off

        if (res.indexOf("OK") != -1)
        {
            state++;
        }
        else
        {
            delay(1000);
        }

        break;
    }

    case 1:
    {
        res = atCmd("AT+QIFGCNT=0");  // select context

        if (res.indexOf("OK") != -1)
        {
            state++;
        }
        else
        {
            delay(1000);
        }

        break;
    }

    case 2:
    {
        res = atCmd("AT+CPIN?");  // is PIN needed?

        if (res.indexOf("SIM PIN") != -1)
        {
            state++;
        }
        else if (res.indexOf("READY") != -1)
        {
            state += 2;
        }
        else
        {
            delay(1000);
        }

        break;
    }

    case 3:
    {
        res = atCmd("AT+CPIN=\"" + PIN + "\"");  // enter PIN

        if (res.indexOf("READY") != -1)
        {
            state++;
        }
        else
        {
            delay(1000);
        }

        break;
    }

    case 4:
    {
        res = atCmd("AT+CGATT=1");  // attach to GRPS service when ready (otherwise 100 error)

        if (res.indexOf("OK") != -1)
        {
            state++;
        }
        else
        {
            delay(1000);
        }

        break;
    }

    case 5:
    {
        res = atCmd("AT+QICSGP=1,\"" + APN + "\"");  // select GPRS as bearer

        if (res.indexOf("OK") != -1)
        {
            state++;
        }
        else
        {
            delay(1000);
        }

        break;
    }

    case 6:
    {
        res = atCmd("AT+QIREGAPP"); // start TCPIP task

        if (res.indexOf("OK") != -1)
        {
            state++;
        }
        else
        {
            delay(1000);
        }

        break;
    }

    case 7:
    {
        res = atCmd("AT+QIACT");  // bring up wireless connection

        if (res.indexOf("OK") != -1)
        {
            isGprsReady = true;
        }
        else
        {
            delay(1000);
        }

        break;
    }
    }
}

void makeHttpGetRequest(String URL)
{
    String res = "";
    char ch;

    switch (reqState)
    {
    case 0:
    {
        res = atCmd("AT+QHTTPURL=" + String(URL.length()) + ",30"); // set URL - url size in bytes, url input timeout

        if(res.indexOf("CONNECT") != -1)
        {
            reqState = 1;
        }
        else
        {
            delay(1000);
        }

        break;
    }

    case 1:
    {
        res = atCmd("http://http.rip"); // enter url

        if(res.indexOf("OK") != -1)
        {
            reqState = 2;
        }
        else
        {
            delay(1000);
        }

        break;
    }

    case 2:
    {
        res = atCmd("AT+QHTTPGET=30");  // make GET request

        if(res.indexOf("OK") != -1)
        {
            reqState = 3;
        }
        else
        {
            delay(1000);
        }

        break;
    }

    case 3:
    {
        res = atCmd("AT+QHTTPREAD=30"); // read results

        Serial.print(res);

        isRequestDone = true;

        break;
    }
    }
}

void loop()
{
    while (!isGprsReady)
    {
        initModem("2159", "internet"); // PIN, APN
    }

    while (!isRequestDone)
    {
        makeHttpGetRequest("http://http.rip"); // URL
    }

    // playground

    while(Serial1.available())
    {
        Serial.write(Serial1.read());
    }

    if (Serial.available())
    {
        String buffer = "";

        while(Serial.available())
        {
            char ch = Serial.read();
            buffer += ch;
            delay(5);
        }

        atCmd(buffer);
    }
}

