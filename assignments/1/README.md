To use this program:
1. Open this directory in your terminal
2. Execute `make`
3. Execute `./chat`
- Note the IP address and port displayed by the program
4. On another machine or seperate terminal window...
- Open this directory
- Run `make`
- Run `./chat -h` to view example client usage
-- Example `./chat -s 122.43.8.139 -p 3490`
5. Take turns sending messages by typing and pressing enter
-- The client must take the first turn

Notes:
Messages must be at most 140 characters long.
I asked if the \0 charcter is included as part of the 140 characters. I didn't get an answer so I assumed it isn't. 

To clean up the object files once you're done, run `make clean`.
