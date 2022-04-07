// Read the charset file
const charset = Deno.readTextFileSync('charset.txt');
const char_regex = /'(.)'\s* = (..)/g

// Match the lines
console.log('export const CHARCODES: Map<number, string> = new Map<number, string>([');

for (const match of charset.matchAll(char_regex)) {
    let char = match[1];
    const code = match[2];

    if (char == '\\') {
        char = '\\\\';
    }

    console.log('    [0x' + code + ', \'' + char + '\'],');
}

console.log(']);');
