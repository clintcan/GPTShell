# GPTShell
This acts as a shell prompt, where you can either type linux commands or with a switch ask chatGPT a question and puts out a response on the screen.  Some code is based off the documentation of OpenAI API, as when experimented, if chatGPT does correct implementation of the API, and somewhat fails in this regard (used GPT-4 as an experiment and the API call created from the prompt needs adjustment).

![image](https://user-images.githubusercontent.com/195128/228692008-db6a08fd-c9b5-4bb6-beb8-64a76f255c84.png)

## How to compile
GPTShell requires the libcurl and rapidjson libraries as dependencies.  On CentOS:

```
$ sudo yum install libcurl-devel
$ sudo yum install rapidjson
```

On Ubuntu/Debian:

```
$ sudo apt-get install rapidjson-dev
$ sudo apt-get install libcurl4-openssl-dev
```
For Debian variants, the deb-multimedia repository has to be enabled to be able to install rapidjson.

## How to Install and Compile
Clone the Directory for GPTShell:

```
$ git clone https://github.com/clintcan/GPTShell.git
$ cd GPTShell
```

Compile the code in the GPTShell directory

```
$ make
$ ./gptshell
```
## Environment Variables and /etc/gptshell
There are currently two variables that are used in GPTShell.  These are the following:
* GPTAPIKEY = this is the OpenAI API Key to use to contact the OpenAI API.  This is required if you want to use the /prompt switch.
* GPTMODEL = this is the model of OpenAI to use as a reference for the /prompt. Optional and defaults to text-davinci-003.

These keys can be stored in environment variables, or for convenience, in /etc/gptshell. The format of /etc/gptshell looks like this:
```
GPTAPIKEY=[your API key]
GPTMODEL=text-davinci-003
```
## Calling ChatGPT through the /prompt switch
By default, any text you type in, and then pressing ENTER will execute the text input through the system() function.  To be able to interact with the OpenAI API, we would start the text with the /prompt switch:

/prompt Question to ask

## Possible Enhancements
execute linux commands without knowing the exact linux command, by running a /do switch. For example:
```
$ /do find all files which has an extension of .txt and contains the word "replace me" in the file
```
do chat sessions so that the /prompt switch will retain previous interactions.
