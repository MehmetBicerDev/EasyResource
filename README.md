# EasyResource Library
EasyResource is a C++ library designed to manage resources by serializing and deserializing them into a binary format. It provides functionality to read, write, and manage resources efficiently.

## Features
Serialize and deserialize resources.
Read resources from binary data.
Save resources to binary files.
Extract resources to the file system.
Manage resource buffers efficiently.
## Dependencies
c_buffer.h (for handling buffer operations)
c_resource.h (for resource definitions)
file_util.h (for file operations)
hash_util.h (for hashing operations)
filesystem (for file system operations, usually part of C++17 and above)
## Usage
Classes and Methods
c_mainresource
This class is the main resource manager, which holds all the resources and file information.

### Members:

std::vector<std::pair<std::string, uint32_t>> m_files: List of files with their CRC hash.
std::vector<c_resource> m_resources: List of resource objects.
data::c_buffer main_buffer: Main buffer to hold resource data.
### Methods:

data::c_buffer serialize(): Serializes the resource information into a buffer.
bool read(data::c_buffer& buffer): Reads resource information from a buffer.
c_resource
This class represents a single resource with its name, size, and buffer.

### Members:

std::string m_name: Resource name.
uint64_t m_size: Size of the resource.
data::c_buffer m_buffer: Buffer holding the resource data.
### Methods:

data::c_buffer serialize_header(): Serializes the resource header (name and size).
data::c_buffer serialize(): Serializes the entire resource.
bool read(data::c_buffer& buffer): Reads the resource data from a buffer.
bool read_header(data::c_buffer& buffer): Reads the resource header from a buffer.
### Functions
`initialize`
Initializes the main resource and loads the resources from the specified path.
```
bool initialize(const std::string& p_mainresourcefilepath);
```

`delete_resource`
Deletes a resource by its index and updates the main buffer.
```
void delete_resource(uint32_t index);
```

`reload_mainbuffer`
Reloads the main buffer with the current resources.
```
void reload_mainbuffer();
```

`load_resources`
Loads resources into the main buffer from the file system.
```
void load_resources();
```

`extract_resources`
Extracts resources from the main buffer to the file system.
```
void extract_resources();
```

`add_file_to_resources`
Adds a file to the resources.
```
void add_file_to_resources(const std::string& file_path, const std::string& resource_name);
```

`save_resources`
Saves the resources to the file system.
```
void save_resources();
```
### Test Functions
`read_test`
Initializes the library, loads resources, and extracts them.

```
void read_test()
{
    easyresource::initialize("Test");
    easyresource::load_resources();

    for (auto m_resource : easyresource::mainresource->m_resources)
    {
        printf("[-] %s : %d \n", m_resource.m_name.c_str(), m_resource.m_size);
    }

    easyresource::extract_resources();
}
```
`write_test`
Initializes the library, adds files as resources, and saves them.
```
void write_test()
{
    printf("Initializing..\n");
    easyresource::initialize("Test");
    printf("adding resources..\n");

    easyresource::add_file_to_resources("Data\\1.jpg", "1.jpg");
    easyresource::add_file_to_resources("Data\\2.jpg", "2.jpg");
    easyresource::add_file_to_resources("Data\\3.webp", "3.webp");
    printf("saving resources..\n");

    easyresource::save_resources();

    printf("done\n");
}
```

## UI Overview
EasyResource comes with a user-friendly graphical interface built using ImGui. This interface simplifies resource management tasks, such as loading, saving, and extracting resources.

### Main Features
Resource Editor: The primary section for managing resources.
File Selection: Choose files to load into the resource manager.
Load and Save Resources: Easily load and save resources to and from the binary format.
Extract Resources: Extract resources to the file system.
Add New Resources: Add new files to the resource manager.
### UI Components
Main Window: The primary interface window without title bar, resize, collapse, or focus options.
Buttons: Interactive buttons for different actions (e.g., Load File, Create Resource, Extract Resource).
Text Display: Shows the status and selected files.
File Selector: A custom file selector for choosing files to manage.
Resource List: Displays the list of resources loaded into the manager.


### UI Workflow
Resource Editor Button: Opens the resource editor section.
Select File Button: Opens the file selector dialog to choose a file.
Load File Button: Loads the selected file into the resource manager.
Create Resource Button: Creates a new resource from the selected file.
Extract Resource Button: Extracts resources to the specified directory.
Save Button: Saves the current state of resources.
Add File Button: Adds a new file to the resources and saves it.
## Building
To build the EasyResource library, ensure you have included all the dependencies and use a C++17 compatible compiler. The following is an example command using g++:

g++ -std=c++17 -o EasyResource main.cpp
Replace main.cpp with the actual source file names.
