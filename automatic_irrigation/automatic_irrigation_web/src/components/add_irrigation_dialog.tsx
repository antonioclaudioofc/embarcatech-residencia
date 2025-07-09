import { Plus } from "lucide-react";
import { useForm } from "react-hook-form";
import { z } from "zod";
import { zodResolver } from "@hookform/resolvers/zod";

import { Label } from "./label";
import { Input } from "./input";
import { Button } from "./button";
import {
  Dialog,
  DialogTrigger,
  DialogContent,
  DialogTitle,
  DialogClose,
  DialogHeader,
  DialogFooter,
} from "./dialog";
import {
  Select,
  SelectTrigger,
  SelectValue,
  SelectContent,
  SelectItem,
} from "./select";

const formSchema = z.object({
  specificDate: z.string().min(1, "Informe a data"),
  times: z
    .array(z.string().min(1, "Horário inválido"))
    .min(1, "Informe pelo menos um horário"),
  duration: z.coerce.number().min(1, "A duração deve ser maior que 0"),
  status: z.enum(["Pendente", "Concluído", "Em andamento"]),
});

type FormData = z.infer<typeof formSchema>;

export function AddIrrigationDialog() {
  const {
    register,
    handleSubmit,
    watch,
    setValue,
    formState: { errors },
  } = useForm<FormData>({
    resolver: zodResolver(formSchema),
    defaultValues: {
      specificDate: "",
      times: [],
      duration: 1,
      status: "Pendente",
    },
  });

  const times = watch("times") || [];

  function addTime() {
    setValue("times", [...times, ""]);
  }

  function updateTime(index: number, value: string) {
    const newTimes = [...times];
    newTimes[index] = value;
    setValue("times", newTimes);
  }

  function removeTime(index: number) {
    const newTimes = times.filter((_, i) => i !== index);
    setValue("times", newTimes);
  }

  function onSubmit(data: FormData) {
    console.log("✅ Dados para enviar:", data);
    // Aqui você pode chamar o fetch/axios para enviar ao backend/Firebase
  }

  return (
    <Dialog>
      <DialogTrigger asChild>
        <Button variant="default" className="gap-2">
          <Plus size={16} />
          Nova irrigação
        </Button>
      </DialogTrigger>
      <DialogContent className="max-w-md">
        <DialogHeader>
          <DialogTitle>Agendar Irrigação</DialogTitle>
        </DialogHeader>
        <form onSubmit={handleSubmit(onSubmit)} className="space-y-4">
          <div>
            <Label>Data específica</Label>
            <Input type="date" {...register("specificDate")} />
            {errors.specificDate && (
              <p className="text-red-500 text-xs mt-1">
                {errors.specificDate.message}
              </p>
            )}
          </div>

          <div>
            <Label>Horários</Label>
            <div className="space-y-2">
              {times.map((time, index) => (
                <div key={index} className="flex items-center gap-2">
                  <Input
                    type="time"
                    value={time}
                    onChange={(e) => updateTime(index, e.target.value)}
                  />
                  <Button
                    type="button"
                    variant="destructive"
                    size="sm"
                    onClick={() => removeTime(index)}
                  >
                    Remover
                  </Button>
                </div>
              ))}
              <Button type="button" variant="outline" onClick={addTime}>
                + Adicionar horário
              </Button>
            </div>
            {errors.times && (
              <p className="text-red-500 text-xs mt-1">
                {errors.times.message}
              </p>
            )}
          </div>

          <div>
            <Label>Duração (min)</Label>
            <Input type="number" {...register("duration")} />
            {errors.duration && (
              <p className="text-red-500 text-xs mt-1">
                {errors.duration.message}
              </p>
            )}
          </div>

          <div>
            <Label>Status</Label>
            <Select
              value={watch("status")}
              onValueChange={(val) =>
                setValue("status", val as FormData["status"])
              }
            >
              <SelectTrigger>
                <SelectValue placeholder="Selecione o status" />
              </SelectTrigger>
              <SelectContent>
                <SelectItem value="Pendente">Pendente</SelectItem>
                <SelectItem value="Em andamento">Em andamento</SelectItem>
                <SelectItem value="Concluído">Concluído</SelectItem>
              </SelectContent>
            </Select>
          </div>

          <DialogFooter className="mt-4">
            <DialogClose asChild>
              <Button type="button" variant="outline">
                Cancelar
              </Button>
            </DialogClose>
            <Button type="submit">Salvar</Button>
          </DialogFooter>
        </form>
      </DialogContent>
    </Dialog>
  );
}
