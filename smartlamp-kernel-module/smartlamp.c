#include <linux/module.h>
#include <linux/usb.h>
#include <linux/slab.h>
#include <linux/string.h>

MODULE_AUTHOR("DevTITANS <devtitans@icomp.ufam.edu.br>");
MODULE_DESCRIPTION("Driver de acesso ao SmartLamp (ESP32 com Chip Serial CP2102");
MODULE_LICENSE("GPL");


#define MAX_RECV_LINE 100 // Tamanho máximo de uma linha de resposta do dispositvo USB


static char recv_line[MAX_RECV_LINE];              // Armazena dados vindos da USB até receber um caractere de nova linha '\n'
static struct usb_device *smartlamp_device;        // Referência para o dispositivo USB
static uint usb_in, usb_out;                       // Endereços das portas de entrada e saida da USB
static char *usb_in_buffer, *usb_out_buffer;       // Buffers de entrada e saída da USB
static int usb_max_size;                           // Tamanho máximo de uma mensagem USB

//do meu esp32 / Ayrton
#define VENDOR_ID 0x1a86  // QinHeng Electronics
#define PRODUCT_ID 0x7523 // CH340 serial converter


static const struct usb_device_id id_table[] = { { USB_DEVICE(VENDOR_ID, PRODUCT_ID) }, {} };

static int  usb_probe(struct usb_interface *ifce, const struct usb_device_id *id); // Executado quando o dispositivo é conectado na USB
static void usb_disconnect(struct usb_interface *ifce);                           // Executado quando o dispositivo USB é desconectado da USB
static int usb_read_serial(void);

// Função para configurar os parâmetros seriais do CP2102 via Control-Messages
static int smartlamp_config_serial(struct usb_device *dev)
{
    int ret;
    u32 baudrate = 9600; // Defina o baud rate que seu ESP32 usa!

    printk(KERN_INFO "SmartLamp: Configurando a porta serial...\n");

    // 1. Habilita a interface UART do CP2102
    //    Comando específico do vendor Silicon Labs (CP210X_IFC_ENABLE)
    //    bmRequestType: 0x41 (Vendor, Host-to-Device, Interface)
    //    bRequest: 0x00 (CP210X_IFC_ENABLE)
    //    wValue: 0x0001 (UART Enable)
    ret = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
                          0x00, 0x41, 0x0001, 0, NULL, 0, 1000);
    if (ret)
    {
        printk(KERN_ERR "SmartLamp: Erro ao habilitar a UART (código %d)\n", ret);
        return ret;
    }

    // 2. Define o baud rate
    //    Comando específico do vendor Silicon Labs (CP210X_SET_BAUDRATE)
    //    bRequest: 0x1E (CP210X_SET_BAUDRATE)
    ret = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
                          0x1E, 0x41, 0, 0, &baudrate, sizeof(baudrate), 1000);
    if (ret < 0)
    {
        printk(KERN_ERR "SmartLamp: Erro ao configurar o baud rate (código %d)\n", ret);
        return ret;
    }

    printk(KERN_INFO "SmartLamp: Baud rate configurado para %d\n", baudrate);
    return 0;
}

// Executado quando o arquivo /sys/kernel/smartlamp/{led, ldr} é lido (e.g., cat /sys/kernel/smartlamp/led)
static ssize_t attr_show(struct kobject *sys_obj, struct kobj_attribute *attr, char *buff);
// Executado quando o arquivo /sys/kernel/smartlamp/{led, ldr} é escrito (e.g., echo "100" | sudo tee -a /sys/kernel/smartlamp/led)
static ssize_t attr_store(struct kobject *sys_obj, struct kobj_attribute *attr, const char *buff, size_t count);
// Variáveis para criar os arquivos no /sys/kernel/smartlamp/{led, ldr}
static struct kobj_attribute  led_attribute = __ATTR(led, S_IRUGO | S_IWUSR, attr_show, attr_store);
static struct kobj_attribute  ldr_attribute = __ATTR(ldr, S_IRUGO | S_IWUSR, attr_show, attr_store);
static struct attribute      *attrs[]       = { &led_attribute.attr, &ldr_attribute.attr, NULL };
static struct attribute_group attr_group    = { .attrs = attrs };
static struct kobject        *sys_obj;                                             // Executado para ler a saida da porta serial

MODULE_DEVICE_TABLE(usb, id_table);

bool ignore = true;
int LDR_value = 0;

static struct usb_driver smartlamp_driver = {
    .name        = "smartlamp",     // Nome do driver
    .probe       = usb_probe,       // Executado quando o dispositivo é conectado na USB
    .disconnect  = usb_disconnect,  // Executado quando o dispositivo é desconectado na USB
    .id_table    = id_table,        // Tabela com o VendorID e ProductID do dispositivo
};

module_usb_driver(smartlamp_driver);

// Executado quando o dispositivo é conectado na USB
static int usb_probe(struct usb_interface *interface, const struct usb_device_id *id) {
    int ret;
    struct usb_endpoint_descriptor *usb_endpoint_in, *usb_endpoint_out;

    printk(KERN_INFO "SmartLamp: Dispositivo conectado ...\n");

    // Cria arquivos do /sys/kernel/smartlamp/*
    sys_obj = kobject_create_and_add("smartlamp", kernel_kobj);
    ignore = sysfs_create_group(sys_obj, &attr_group); // AQUI

    // Detecta portas e aloca buffers de entrada e saída de dados na USB
    smartlamp_device = interface_to_usbdev(interface);
    ignore =  usb_find_common_endpoints(interface->cur_altsetting, &usb_endpoint_in, &usb_endpoint_out, NULL, NULL);  // AQUI
    usb_max_size = usb_endpoint_maxp(usb_endpoint_in);
    usb_in = usb_endpoint_in->bEndpointAddress;
    usb_out = usb_endpoint_out->bEndpointAddress;
    usb_in_buffer = kmalloc(usb_max_size, GFP_KERNEL);
    usb_out_buffer = kmalloc(usb_max_size, GFP_KERNEL);

    // Chama a função para configurar a porta serial antes de usar
    ret = smartlamp_config_serial(smartlamp_device);
    if (ret)
    {
        printk(KERN_ERR "SmartLamp: Falha na configuração da serial\n");
        kfree(usb_in_buffer);
        kfree(usb_out_buffer);
        sysfs_remove_group(sys_obj, &attr_group);
        kobject_put(sys_obj);
        return ret;
    }

    LDR_value = usb_read_serial();

    printk("LDR Value: %d\n", LDR_value);

    return 0;
}

// Executado quando o dispositivo USB é desconectado da USB
static void usb_disconnect(struct usb_interface *interface) {
    printk(KERN_INFO "SmartLamp: Dispositivo desconectado.\n");
    if (sys_obj) kobject_put(sys_obj);      // Remove os arquivos em /sys/kernel/smartlamp
    kfree(usb_in_buffer);                   // Desaloca buffers
    kfree(usb_out_buffer);
}

// Envia um comando via USB, espera e retorna a resposta do dispositivo (convertido para int)
// Exemplo de Comando:  SET_LED 80
// Exemplo de Resposta: RES SET_LED 1
// Exemplo de chamada da função usb_send_cmd para SET_LED: usb_send_cmd("SET_LED", 80);
static int usb_send_cmd(char *cmd, int param) {
    int ret, actual_size;
    int retries = 10;
    long resp_number = -1;

    // 1. Formata o comando com o parâmetro no buffer de saída
    if (param != -1) {
        sprintf(usb_out_buffer, "%s %d\n", cmd, param);
    } else {
        sprintf(usb_out_buffer, "%s\n", cmd);
    }

    printk(KERN_INFO "SmartLamp: Enviando comando: %s", usb_out_buffer);

    // 2. Envia o comando formatado para a USB
    ret = usb_bulk_msg(smartlamp_device, 
                       usb_sndbulkpipe(smartlamp_device, usb_out), 
                       usb_out_buffer, 
                       strlen(usb_out_buffer), 
                       &actual_size, 
                       1000);
    if (ret) {
        printk(KERN_ERR "SmartLamp: Erro de codigo %d ao enviar comando!\n", ret);
        return -1;
    }

    // 3. Loop para ler a resposta
    while (retries > 0) {
        ret = usb_bulk_msg(smartlamp_device, 
                           usb_rcvbulkpipe(smartlamp_device, usb_in), 
                           usb_in_buffer, 
                           usb_max_size, 
                           &actual_size, 
                           1000);
        if (ret) {
            printk(KERN_ERR "SmartLamp: Erro ao ler da USB (tentativa %d). Codigo: %d\n", retries--, ret);
            msleep(100); // Espera um pouco antes de tentar de novo
            continue;
        }

        // Garante que a string lida termina com NULL
        usb_in_buffer[actual_size] = '\0';
        printk(KERN_INFO "SmartLamp: Recebido: \"%s\"", usb_in_buffer);

        // Procura pela resposta esperada, que geralmente começa com "RES"
        if (strstr(usb_in_buffer, "RES")) {
            // Tenta extrair um número da resposta
            sscanf(usb_in_buffer, "RES %*s %ld", &resp_number);
            printk(KERN_INFO "SmartLamp: Numero extraido da resposta: %ld\n", resp_number);
            return resp_number;
        }

        retries--;
    }

    printk(KERN_ERR "SmartLamp: Nao recebi a resposta esperada do dispositivo.\n");
    return -1;
}

// Executado quando o arquivo /sys/kernel/smartlamp/{led, ldr} é lido (e.g., cat /sys/kernel/smartlamp/led)
static ssize_t attr_show(struct kobject *sys_obj, struct kobj_attribute *attr, char *buff) {
    int value;
    const char *attr_name = attr->attr.name;

    printk(KERN_INFO "SmartLamp: Lendo %s ...\n", attr_name);

    if (strcmp(attr_name, "ldr") == 0) {
        // Envia o comando para ler o LDR. O segundo parâmetro é ignorado.
        value = usb_send_cmd("GET_LDR", -1);
    }
    else if (strcmp(attr_name, "led") == 0) {
        // Envia o comando para ler o estado do LED.
        value = usb_send_cmd("GET_LED", -1);
    }
    else {
        printk(KERN_ERR "SmartLamp: Atributo desconhecido para leitura: %s\n", attr_name);
        return -EINVAL;
    }

    if (value < 0) {
        printk(KERN_ERR "SmartLamp: Falha ao obter valor para %s\n", attr_name);
        // Retorna um erro para o userspace, ex: "cat: read error: Input/output error"
        return -EIO;
    }

    sprintf(buff, "%d\n", value);
    return strlen(buff);
}


// Essa função não deve ser alterada durante a task sysfs
// Executado quando o arquivo /sys/kernel/smartlamp/{led, ldr} é escrito (e.g., echo "100" | sudo tee -a /sys/kernel/smartlamp/led)
static ssize_t attr_store(struct kobject *sys_obj, struct kobj_attribute *attr, const char *buff, size_t count) {
    long ret, value;
    const char *attr_name = attr->attr.name;

    if (strcmp(attr_name, "ldr") == 0) {
        printk(KERN_ERR "SmartLamp: O atributo 'ldr' é somente leitura.\n");
        return -EPERM; // Operation not permitted
    }

    ret = kstrtol(buff, 10, &value);
    if (ret) {
        printk(KERN_ALERT "SmartLamp: valor de %s invalido.\n", attr_name);
        return -EINVAL; // Invalid argument
    }

    printk(KERN_INFO "SmartLamp: Setando %s para %ld ...\n", attr_name, value);

    // Envia o comando para definir o valor do LED
    ret = usb_send_cmd("SET_LED", value);

    if (ret < 0) {
        printk(KERN_ALERT "SmartLamp: erro ao setar o valor do %s.\n", attr_name);
        return -EIO; // Input/output error
    }

    return count; // Retorna o número de bytes escritos com sucesso
}


static int usb_read_serial(void) {
    int ret, actual_size;
    // Tenta ler dados da porta de entrada USB
    ret = usb_bulk_msg(smartlamp_device, 
                       usb_rcvbulkpipe(smartlamp_device, usb_in), 
                       usb_in_buffer, 
                       min(usb_max_size, MAX_RECV_LINE), 
                       &actual_size, 
                       1000);
    if (ret) {
        printk(KERN_ERR "SmartLamp: Erro ao ler da USB no probe (código %d)\n", ret);
        return -1;
    }
    // Apenas para debug, podemos imprimir o que foi lido
    printk(KERN_INFO "SmartLamp: Lido na inicialização: %.*s\n", actual_size, usb_in_buffer);
    
    // ATENÇÃO: Esta função precisaria de uma lógica para extrair um número
    // do buffer, mas para o probe inicial, podemos retornar um valor fixo.
    return 0; // Ou um valor de LDR padrão
}