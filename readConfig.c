#include "readConfig.h"



void readConfig()
{
	int show = 1;  // turn on off debugging


    FILE *config;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    config = fopen("/etc/tc/tcconfig.txt","r");
    Boolean alt = True;
    
    char *data = NULL;
    while((read = getline(&line,&len,config)) != -1)
    {
	    debugLogging(show,False,"%s\n",line);	
		if(strncmp(line,"alt",3)==0)
		{
			debugLogging(show,False,"alt\n");
			alt = True;
		}
		else if(strncmp(line,"az",2)==0)
		{
			debugLogging(show,False,"az\n");
			alt = False;
		}
		else if(strncmp(line,"bigGear",7)==0)
		{
			debugLogging(show,False,"bigGear\n");
			char s[2] = "=";
			data = strtok(line,s);
			if(data!= NULL)
			{
			   data = strtok(NULL,s);
			   if(alt) 
			   {
				   sscanf(data,"%ld",&altBigTooth);
			   }
			   else 
			   {
				   sscanf(data,"%ld",&azBigTooth);
			   }
		    }
		}
		else if(strncmp(line,"smallGear",9)==0)
		{
			debugLogging(show,False,"smallGear\n");
			char s[2] = "=";
			data = strtok(line,s);
			if(data!= NULL)
			{
			   data = strtok(NULL,s);
			   if(alt) 
			   {
				   sscanf(data,"%ld",&altSmallTooth);
			   }
			   else 
			   {
				   sscanf(data,"%ld",&azSmallTooth);
			   }
			}
		}
		else if(strncmp(line,"pulseInverted",13)==0)
		{
			debugLogging(show,False,"pulseInverted\n");
			if(alt) 
			{
				invertAltPulse = 1;
			}
			else
			{
				invertAzPulse = 1;
			}
		}
		else if(strncmp(line,"directionInverted",17)==0)
		{
			debugLogging(show,False,"directionInverted\n");
			if(alt) 
			{
				invertAltDirection = 1;
			}
			else
			{
				invertAzDirection = 1;
			}
		}
		else if(strncmp(line,"enableInverted",14)==0)
		{
			debugLogging(show,False,"enableInverted\n");
			if(alt) 
			{
				invertAltEnable = 1;
			}
			else
			{
				invertAzEnable = 1;
			}
		}
		else if(strncmp(line,"stepsPerRevolution",18)==0)
		{
			debugLogging(show,False,"stepsPerRevolution\n");
			char s[2] = "=";
			data = strtok(line,s);
			if(data!= NULL)
			{
			   data = strtok(NULL,s);
			   if(alt) 
			   {
				   sscanf(data,"%ld",&altStepsPerRev);
			   }
			   else 
			   {
				   sscanf(data,"%ld",&azStepsPerRev);
			   }
			}
		}
		else if(strncmp(line,"gamepad",7)==0)
		{
			debugLogging(show,False,"gamepad\n");
			char s[2] = "=";
			data = strtok(line,s);
			if(data!= NULL)
			{
			   data = strtok(NULL,s);
			   sscanf(data,"%s",gamepadFile);
			   debugLogging(show,False,"gamepad %s\n",gamepadFile);
			}
		}
		else if(strncmp(line,"gotoTimer",9)==0)
		{
			debugLogging(show,False,"gotoTiemr\n");
			char s[2] = "=";
			data = strtok(line,s);
			if(data!= NULL)
			{
			   data = strtok(NULL,s);
			   sscanf(data,"%d",&gotoTimer);
			   debugLogging(show,False,"gotoTimer %d\n",gotoTimer);
			}
		}
		else if(strncmp(line,"joyKeySteps",11)==0)
		{
			debugLogging(show,False,"joyKeySteps\n");
			char s[2] = "=";
			data = strtok(line,s);
			if(data!= NULL)
			{
			   if(alt) 
			   {
				   data = strtok(NULL,s);
				   sscanf(data,"%d",&altJoyKeySteps);
				   debugLogging(show,False,"joykey1 %d\n",altJoyKeySteps);
			   }
			   else
			   {
				   data = strtok(NULL,s);
				   sscanf(data,"%d",&azJoyKeySteps);
				   debugLogging(show,False,"joykey2 %d\n",azJoyKeySteps);
			   }
			}
		}
		else if(strncmp(line,"buttonKeySteps",14)==0)
		{
			debugLogging(show,False,"buttonKeySteps\n");
			char s[2] = "=";
			data = strtok(line,s);
			if(data!= NULL)
			{
			   if(alt) 
			   {
				   data = strtok(NULL,s);
				   sscanf(data,"%d",&altButtonKeySteps);
				   debugLogging(show,False,"buttonKey1 %d\n",altButtonKeySteps);
			   }
			   else
			   {
				   data = strtok(NULL,s);
				   sscanf(data,"%d",&azButtonKeySteps);
				   debugLogging(show,False,"buttonkey2 %d\n",azButtonKeySteps);
			   }
			}
		}
		else if(strncmp(line,"setupTimer",10)==0)
		{
			debugLogging(show,False,"setupTimer\n");
			char s[2] = "=";
			data = strtok(line,s);
			if(data!= NULL)
			{ 
				data = strtok(NULL,s);
			    debugLogging(show,False,"setupTimer %s\n",data);
				sscanf(data,"%d",&setupTimer);
			    debugLogging(show,False,"setupTimer %d\n",setupTimer);
			}
		}
		else if(strncmp(line,"slowRatio",9)==0)
		{
			debugLogging(show,False,"slowRatio\n");
			char s[2] = "=";
			data = strtok(line,s);
			if(data!= NULL)
			{
				data = strtok(NULL,s);
				if(alt)
				{
					sscanf(data,"%d",&slowAltRatio);
					debugLogging(show,False,"slowAltRatio %d\n",slowAltRatio);
				}
				else
				{
					sscanf(data,"%d",&slowAzRatio);
					debugLogging(show,False,"slowAzRatio %d\n",slowAzRatio);
				}
			}
		}
		else if(strncmp(line,"aRatio",6)==0)
		{
			debugLogging(show,False,"aRatio\n");
			char s[2] = "=";
			data = strtok(line,s);
			if(data!= NULL)
			{
		        data = strtok(NULL,s);
				if(alt)
				{
					sscanf(data,"%d",&aAltRatio);
					debugLogging(show,False,"aAltRatio %d\n",aAltRatio);
				}
				else 
				{
					sscanf(data,"%d",&aAzRatio);
					debugLogging(show,False,"aAzRatio %d\n",aAltRatio);
				}
			}
		}
		else if(strncmp(line,"bRatio",6)==0)
		{
			debugLogging(show,False,"bRatio\n");
			char s[2] = "=";
			data = strtok(line,s);
			if(data!= NULL)
			{
		        data = strtok(NULL,s);
				if(alt)
				{
					sscanf(data,"%d",&bAltRatio);
					debugLogging(show,False,"bAltRatio %d\n",bAltRatio);
				}
				else 
				{	
					sscanf(data,"%d",&bAzRatio);
					debugLogging(show,False,"bAzRatio %d\n",bAltRatio);
				}
			}
		}
		else if(strncmp(line,"xRatio",6)==0)
		{
			debugLogging(show,False,"xRatio\n");
			char s[2] = "=";
			data = strtok(line,s);
			if(data!= NULL)
			{
		        data = strtok(NULL,s);
				if(alt)
				{
					sscanf(data,"%d",&xAltRatio);
					debugLogging(show,False,"xAltRatio %d\n",xAltRatio);
				}
				else 
				{
					sscanf(data,"%d",&xAzRatio);
					debugLogging(show,False,"xAzRatio %d\n",xAltRatio);
				}
			}
		}
		else if(strncmp(line,"yRatio",6)==0)
		{
			debugLogging(show,False,"yRatio\n");
			char s[2] = "=";
			data = strtok(line,s);
			if(data!= NULL)
			{
		        data = strtok(NULL,s);
				if(alt)
				{
					sscanf(data,"%d",&yAltRatio);
					debugLogging(show,False,"yAltRatio %d\n",yAltRatio);
				}
				else 
				{
					sscanf(data,"%d",&yAzRatio);
					debugLogging(show,False,"yAzRatio %d\n",yAltRatio);
				}
			}
		}
            	else if(strncmp(line,"correctionPercent",9)==0)
		{
			debugLogging(show,False,"correctionPrecent\n");
			char s[2] = "=";
			data = strtok(line,s);
			if(data!= NULL)
			{
		        data = strtok(NULL,s);
				if(alt)
				{
					sscanf(data,"%f",&correctionPercentAlt);
					debugLogging(show,False,"correctionPercent %f\r\n",correctionPercentAlt);
				}
				else 
				{
					sscanf(data,"%f",&correctionPercentAz);
					debugLogging(show,False,"correctionPercent %f\r\n",correctionPercentAz);
				}
			}
		}

        else if(strncmp(line,"typegamepad",11) == 0)
        {
			debugLogging(show,False,"typegamepad\n");
			char s[2] = "=";
			data = strtok(line,s);
			if(data!=NULL)
			{
				data = strtok(NULL,s);
				if(strncmp(data,"fx",2)==0)
				{
					typeGamepad = FX;
				}
				else
				{
					typeGamepad = SNES;
				}
			}
		}
    }
    
    free(line);
    fclose(config);
}
