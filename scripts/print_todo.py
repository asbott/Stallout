import re
import os
import argparse

def find_todos_in_file(file_path):
    with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
        lines = f.readlines()

    capturing = False
    todos = []
    current_tags = []
    current_description = []

    for line_number, line in enumerate(lines, 1):
        stripped_line = line.strip()

        if "TODO" in stripped_line.upper():
            capturing = True
            current_description.append(re.sub(r"//|/\*|\*/|TODO:?", "", stripped_line).strip())
            continue
        
        if capturing:
            if stripped_line.startswith("//") or "/*" in stripped_line or "*/" in stripped_line:
                current_description.append(re.sub(r"//|/\*|\*/", "", stripped_line).strip())
            else:
                description_text = "\n".join(current_description)
                current_tags = re.findall(r"#\w+", description_text)
                for tag in current_tags:
                    description_text = description_text.replace(tag, "").strip()
                todos.append((line_number, current_tags, description_text))
                current_tags = []
                current_description = []
                capturing = False

    if capturing:
        description_text = "\n".join(current_description)
        current_tags = re.findall(r"#\w+", description_text)
        for tag in current_tags:
            description_text = description_text.replace(tag, "").strip()
        todos.append((line_number-1, current_tags, description_text))

    return todos

def main(args):
    files = [
        os.path.join(dp, f) for dp, dn, filenames in os.walk(args.directory) 
        for f in filenames 
        if os.path.splitext(f)[1] in ['.c', '.cpp', '.h', '.hpp'] 
        and (not args.filename_filter or f.startswith(args.filename_filter)) 
        and (not args.directory_filter or any(d_filter in dp for d_filter in args.directory_filter))
    ]
    total_todos = 0
    todos_per_tag = {}
    did_print = False
    for file_path in files:
        todos = find_todos_in_file(file_path)
        
        total_todos += len(todos)
        if len(todos) > 0 and not did_print:
            print(f"\n========={args.directory}=========\n")
            did_print = True
        
        for line_number, tags, description in todos:
            for tag in tags:
                if tag in todos_per_tag:
                    todos_per_tag[tag] += 1
                else:
                    todos_per_tag[tag] = 1
            if not args.tags_filter or any(tag in tags for tag in args.tags_filter):
                print(f"File: {os.path.basename(file_path)}")
                print(f"Line {line_number}")
                if tags:
                    print("Tags:", ' '.join(tags))
                else:
                    print("No Tags")
                if description.strip():
                    print("Description:", description.strip())
                else:
                    print("No Description")
                print("----------------------------------------")
    print("\nTotal TODOs:", total_todos)
    print("TODOs per Tag:")
    for tag, count in todos_per_tag.items():
        print(f"{tag}: {count}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Find TODO comments in code files.")
    parser.add_argument("directory", help="Path of the directory to search in.")
    parser.add_argument("--tags_filter", "-t", nargs='*', help="Filter the results by these tags. Multiple tags are allowed.")
    parser.add_argument("--filename_filter", "-f", default="", help="Filter files based on the start of the filename.")
    parser.add_argument("--directory_filter", "-d", nargs='*', default=[], help="Filter files based on these directory names. Multiple directories are allowed.")
    
    args = parser.parse_args()
    main(args)