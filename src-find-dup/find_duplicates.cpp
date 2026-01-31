#include "DarkMark.hpp"


std::string find_oldest_file(const dm::SStr & filenames)
{
	Time t = Time::getCurrentTime();
	std::string oldest_filename;

	for (const auto & fn : filenames)
	{
		File f(fn);
		if (f.getCreationTime() < t)
		{
			// this file is older, remember the time and name
			t = f.getCreationTime();
			oldest_filename = fn;
		}
	}

	return oldest_filename;
}


void find_md5(dm::MStr & filenames, std::atomic<size_t> & file_counter)
{
	// many copies of this are started, each on a new thread

	std::string most_recent_filename = "?";

	try
	{
		for (auto & [fn, checksum] : filenames)
		{
			most_recent_filename = fn;
			File f(fn);
			MD5 md5(f);
			filenames[fn] = md5.toHexString().toStdString();
			file_counter ++;
		}
	}
	catch (const std::exception & e)
	{
		std::cout << "ERROR while processing " << most_recent_filename << ": " << e.what() << std::endl;
	}

	return;
}


int main(int argc, char * argv[])
{
	int rc = 1;

	try
	{
		if (argc < 2)
		{
			std::cout
				<< "Recursively check images in a dataset to find duplicates." << std::endl
				<< "This uses the MD5 checksum of the image files, so only exact duplicates will be found." << std::endl
				<< "" << std::endl
				<< "Example 1:  " << argv[0] << " ~/nn/cars/set_03/ ~/nn/cars/set_05/" << std::endl
				<< "Example 2:  " << argv[0] << " ." << std::endl;

			throw std::invalid_argument("no subdirectory specified");
		}

		const dm::SStr extensions_of_interest =
		{
			".jpg",
			".jpeg",
			".gif",
			".png",
			".tiff",
			".webp",
			".avi",
			".mv4",
			".m4v",
			".mp4",
			".mpeg",
			".mjpeg",
			".mov"
		};
		std::cout << "File extensions to check ....................";
		for (const auto & ext : extensions_of_interest)
		{
			std::cout << " " << ext;
		}
		std::cout << std::endl;

		dm::SStr all_directories;
		dm::MStr all_files_and_md5s;
		size_t files_skipped = 0;

		for (int i = 1; i < argc; i ++)
		{
			File f(argv[i]);
			if (not f.exists())
			{
				throw std::invalid_argument("\"" + f.getFullPathName().toStdString() + "\" does not exist");
			}

			if (f.isDirectory())
			{
				all_directories.insert(f.getFullPathName().toStdString());
			}
			else
			{
				all_files_and_md5s[f.getFullPathName().toStdString()] = "";
			}
		}

		std::cout << "Number of initial directories to scan ....... " << all_directories.size() << std::endl;
		for (const auto & dir_name : all_directories)
		{
			std::cout << "Scanning directory .......................... " << dir_name << std::endl;

			File dir(dir_name);
			auto files = dir.findChildFiles(File::TypesOfFileToFind::findFiles + File::TypesOfFileToFind::ignoreHiddenFiles, true);
			for (const auto & file : files)
			{
				if (extensions_of_interest.count(file.getFileExtension().toLowerCase().toStdString()) == 1)
				{
					all_files_and_md5s[file.getFullPathName().toStdString()] = "";
				}
				else
				{
					files_skipped ++;
				}
			}
		}

		std::cout
			<< "Files skipped (unknown extension) ........... " << files_skipped << std::endl
			<< "Number of image and video files to verify ... " << all_files_and_md5s.size() << std::endl;

		// split the set of files into multiple sets so we can get multiple threads working on this at the same time
		size_t nproc = std::max(1U, std::thread::hardware_concurrency());
		std::vector<dm::MStr> files_and_md5s(nproc);
		int counter = 0;
		for (const auto & [fn, md5] : all_files_and_md5s)
		{
			files_and_md5s[counter % nproc][fn] = "";
			counter ++;
		}
		std::cout << "Number of threads to create ................. " << files_and_md5s.size() << " with " << files_and_md5s[0].size() << " files each" << std::endl;

		// this is where we start the threads that will inspect each of the images

		dm::VThreads threads;
		std::atomic<size_t> file_counter = 0;
		for (auto & s : files_and_md5s)
		{
			threads.emplace_back(std::thread(find_md5, std::ref(s), std::ref(file_counter)));
		}
		while (file_counter < all_files_and_md5s.size())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(750));
			std::cout << "\rCalculating the MD5 checksum of all files ... " << (int)std::round(100.0f * file_counter / all_files_and_md5s.size()) << "% " << std::flush;
		}
		std::cout << std::endl;
		for (auto & t : threads)
		{
			t.join();
		}

		// put the individual thread results back into a single map
		for (auto & m : files_and_md5s)
		{
			for (const auto & [fn, md5] : m)
			{
				all_files_and_md5s[fn] = md5;
			}
		}
		files_and_md5s.clear();

		// now we look for duplicates
		dm::SStr all_md5s;
		dm::SStr duplicate_md5s;
		size_t count_duplicate_files = 0;
		for (auto & [fn, md5] : all_files_and_md5s)
		{
			if ((all_md5s.size() == all_files_and_md5s.size() - 1) or
				(all_md5s.size() % 100 == 0))
			{
				std::cout << "\rLooking for duplicate MD5 checksums ......... " << (int)std::round(100.0f * all_md5s.size() / all_files_and_md5s.size()) << "% " << std::flush;
			}
			if (all_md5s.count(md5) == 0)
			{
				all_md5s.insert(md5);
				continue;
			}

			// if we get here, then we have a duplicate!
			count_duplicate_files ++;

			if (duplicate_md5s.count(md5) == 0)
			{
				// this is a new duplicate (so count the origin as a duplicate as well)
				count_duplicate_files ++;
				duplicate_md5s.insert(md5);
			}
		}
		std::cout << std::endl	<< "Number of duplicate MD5 checksums ........... " << duplicate_md5s.size() << std::endl
								<< "Number of duplicate files ................... " << count_duplicate_files << std::endl;

		// list all duplicates
		dm::SStr simple_delete_solution;
		for (const auto & duplicate_md5 : duplicate_md5s)
		{
			dm::SStr similar_files_without_annotations;
			dm::SStr similar_files_with_annotations;
			dm::SStr similar_files_negative_samples;

			std::cout << std::endl << duplicate_md5 << ":" << std::endl;
			for (const auto & [fn, md5] : all_files_and_md5s)
			{
				if (duplicate_md5 == md5)
				{
					std::cout << "-> " << fn;

					File f = File(fn).withFileExtension(".txt");
					if (f.existsAsFile())
					{
						StringArray a;
						f.readLines(a);
						if (a.size() == 0)
						{
							std::cout << "\x1b[1;37m [negative sample]\x1b[0m";
							similar_files_negative_samples.insert(fn);
						}
						else if (a.size() == 1)
						{
							std::cout << "\x1b[1;37m [1 annotation]\x1b[0m";
							similar_files_with_annotations.insert(fn);
						}
						else
						{
							std::cout << "\x1b[1;37m [" << a.size() << " annotations]\x1b[0m";
							similar_files_with_annotations.insert(fn);
						}
					}
					else
					{
						similar_files_without_annotations.insert(fn);
					}

					std::cout << std::endl;
				}
			}

#if 0
			for (const auto & f : similar_files_negative_samples)		std::cout << "NEGATIVE SAMPLES: " << f << std::endl;
			for (const auto & f : similar_files_with_annotations)		std::cout << "WITH ANNOTATIONS: " << f << std::endl;
			for (const auto & f : similar_files_without_annotations)	std::cout << "ZERO ANNOTATIONS: " << f << std::endl;
#endif

			// see if we can tell the user which file needs to be deleted

			if (similar_files_without_annotations.size() > 0 and (similar_files_with_annotations.size() > 0 or similar_files_negative_samples.size() > 0))
			{
				// first case -- if we have annotations, then all of the ones without annotations can be deleted
				for (const auto & fn : similar_files_without_annotations)
				{
					simple_delete_solution.insert(fn);
				}
			}
			else if (similar_files_negative_samples.size() > 0 and similar_files_with_annotations.size() == 0 and similar_files_without_annotations.size() == 0)
			{
				// next case -- we ONLY have negative samples, so keep the oldest file

				const std::string oldest_file_in_set = find_oldest_file(similar_files_negative_samples);
				for (const auto & fn : similar_files_negative_samples)
				{
					if (fn != oldest_file_in_set)
					{
						simple_delete_solution.insert(fn);
					}
				}
			}
			else if (similar_files_without_annotations.size() > 0 and similar_files_with_annotations.size() == 0 and similar_files_negative_samples.size() == 0)
			{
				// next case -- we ONLY have non-annotated versions of this file, in which case we'll keep the oldest file

				const std::string oldest_file_in_set = find_oldest_file(similar_files_without_annotations);

				for (const auto & fn : similar_files_without_annotations)
				{
					if (fn != oldest_file_in_set)
					{
						simple_delete_solution.insert(fn);
					}
				}
			}
		}

		if (simple_delete_solution.size() > 0)
		{
			std::cout << std::endl << "\x1b[1;31mWARNING:  running the following commands will DELETE files from disk!\x1b[0m" << std::endl;

			for (const auto & fn : simple_delete_solution)
			{
				std::cout << "\x1b[1;33mrm \"" << fn << "\"\x1b[0m" << std::endl;

				// see if we have annotation files to delete as well
				for (const auto & ext : {".txt", ".json"})
				{
					File f = File(fn).withFileExtension(ext);
					if (f.existsAsFile())
					{
						std::cout << "\x1b[1;33mrm \"" << f.getFullPathName() << "\"\x1b[0m" << std::endl;
					}
				}
			}

			std::cout
					<< std::endl
					<< "Number of duplicate MD5 checksums ........... " << duplicate_md5s.size() << std::endl
					<< "Number of duplicate files ................... " << count_duplicate_files << std::endl
					<< "Number of simple source files to delete ..... " << simple_delete_solution.size() << std::endl
					<< std::endl;
		}

		rc = 0;
	}
	catch (const std::exception & e)
	{
		std::cout << std::endl << "ERROR: " << e.what() << std::endl;
		rc = 2;
	}

	return rc;
}
