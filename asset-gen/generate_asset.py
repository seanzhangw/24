from PIL import Image
# 8, 12
L_ratio = 6
W_ratio = 10

def simplify_to_primary(image_path):
    img = Image.open(image_path).convert('RGB')
    width, height = img.size
    new_width = width // W_ratio
    new_height = height // L_ratio

    pixels = [[0x00 for _ in range(new_width)] for _ in range(new_height)]

    def get_color_byte(r, g, b):
        if r < 50 and g < 50 and b < 50:
            return 0x00  # Black
        elif r > 200 and g > 200 and b > 200:
            return 0xFF  # White
        elif r > g and r > b:
            return 0x88  # Red (or you can use another value)
        else:
            return 0xFF  # Default to white

    for y in range(new_height):
        for x in range(new_width):
            if x == 0 or y == 0 or x == new_width - 1 or y == new_height - 1:
                pixels[y][x] = 0xFF
            else:
                r, g, b = img.getpixel((x * W_ratio, y * L_ratio))
                pixels[y][x] = get_color_byte(r, g, b)

    # Print nicely formatted
    for row in pixels:
        print("{" + ", ".join(f"0x{val:02X}" for val in row) + "},")
    print(len(pixels)), print(len(pixels[0]))

# Example usage:
image_path = "./content/9_of_clubs.png"  # Replace with your image path
simplify_to_primary(image_path)
