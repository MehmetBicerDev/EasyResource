#include <iostream>
#include <easyresource.h>

#pragma comment(lib, "EasyResource")


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
int main()
{
   // read_test();
    write_test();
    system("pause");
}

