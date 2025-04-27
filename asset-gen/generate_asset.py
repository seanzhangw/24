from PIL import Image
L_ratio = 7
W_ratio = 12

def getPixels(image_path):
    img = Image.open(image_path).convert('RGB')
    width, height = img.size
    new_width = width // W_ratio
    new_height = height // L_ratio
    print(f"Original size: {width}x{height}, New size: {new_width}x{new_height}")

    pixels = [[0x00 for _ in range(new_width)] for _ in range(new_height)]

    def get_color_byte(r, g, b):
        if r < 50 and g < 50 and b < 50:
            return 0x00  # Black
        elif r > 200 and g > 200 and b > 200:
            return 0xFF  # White
        elif r > g and r > b:
            return 0x88  # Red
        else:
            return 0x66  # Default is Blue

    for y in range(new_height):
        for x in range(new_width):
            if x == 0 or y == 0 or x == new_width - 1 or y == new_height - 1:
                pixels[y][x] = 0xFF
            else:
                r, g, b = img.getpixel((x * W_ratio, y * L_ratio))
                pixels[y][x] = get_color_byte(r, g, b)

    # Array Format
    for row in pixels[2:-3]:
        print("{" + ", ".join(f"0x{val:02X}" for val in row[1:-1]) + "},")

image_path = "./content/back.png"
getPixels(image_path)