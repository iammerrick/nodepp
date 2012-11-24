
This project is meant to be a library which permits C++ coders to take advantage
of the asynchronous event-loop lambda-based IO approach made populate by
Node.js.  At the moment, this is simply a toy project that I am playing with. I
am happy to accept any assistance or hear of any interest.

Here is an example of what this library will potentially allow you to do:

    int main() {
      nodepp::Core core;
      nodepp::FileSystem fs(core);

      core.start([]() {
        fs.open("some_file.txt",[](Fd fd) {
          fd.readLines([]( string line ) {
            // do something for each line of file
            cout << line << endl;
          });
        });
      });
    }


