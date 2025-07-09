import { useEffect, useState } from "react";
import { Card, CardContent } from "../components/card";
import {
  Table,
  TableHeader,
  TableRow,
  TableHead,
  TableBody,
  TableCell,
} from "../components/table";
import { Badge } from "../components/badge";
import { Button } from "../components/button";
import { Pencil, Trash2, Plus } from "lucide-react";
import { AddIrrigationDialog } from "../components/add_irrigation_dialog";

type Status = "Pendente" | "Concluído" | "Em andamento";

type IrrigationItem = {
  specificDate: string;
  times: string[];
  duration: number;
  status: Status;
};

const mockData: Record<string, IrrigationItem> = {
  id1: {
    specificDate: "2025-07-09",
    times: ["18:00", "20:00", "21:00"],
    duration: 1,
    status: "Pendente",
  },
  id2: {
    specificDate: "2025-07-10",
    times: ["06:00"],
    duration: 5,
    status: "Em andamento",
  },
  id3: {
    specificDate: "2025-07-11",
    times: ["12:00", "14:30"],
    duration: 3,
    status: "Concluído",
  },
};

export function Dashboard() {
  const [data, setData] = useState<Record<string, IrrigationItem>>({});
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    const timer = setTimeout(() => {
      setData(mockData);
      setLoading(false);
    }, 1000);

    return () => clearTimeout(timer);
  }, []);

  return (
    <Card className="border-none shadow-none">
      <CardContent className="py-6 px-4 space-y-4 max-w-7xl m-auto">
        <div className="flex flex-wrap items-center justify-between">
          <h2 className="text-3xl font-bold max-md:text-center max-md:w-full">
            Agendamentos de Irrigação
          </h2>
          <AddIrrigationDialog />
        </div>

        {loading ? (
          <p className="text-sm text-gray-500">Carregando dados...</p>
        ) : (
          <>
            {/* Versão Mobile */}
            <div className="lg:hidden space-y-4">
              {Object.entries(data).map(([id, item]) => (
                <div
                  key={id}
                  className="border p-4 rounded-lg shadow-sm bg-gray-50"
                >
                  <p>
                    <strong>Data:</strong> {item.specificDate}
                  </p>
                  <p>
                    <strong>Horários:</strong> {item.times.join(", ")}
                  </p>
                  <p>
                    <strong>Duração:</strong> {item.duration} min
                  </p>
                  <p>
                    <strong>Status:</strong> {item.status}
                  </p>
                  <div className="flex justify-end gap-2 mt-2">
                    <Button size="icon" variant="outline">
                      <Pencil size={16} />
                    </Button>
                    <Button size="icon" variant="destructive">
                      <Trash2 size={16} />
                    </Button>
                  </div>
                </div>
              ))}
            </div>

            {/* Versão Desktop */}
            <div className="hidden lg:block">
              <Table>
                <TableHeader>
                  <TableRow>
                    <TableHead>Data</TableHead>
                    <TableHead>Horários</TableHead>
                    <TableHead>Duração (min)</TableHead>
                    <TableHead>Status</TableHead>
                    <TableHead className="text-right">Ações</TableHead>
                  </TableRow>
                </TableHeader>
                <TableBody>
                  {Object.entries(data).map(([id, item]) => (
                    <TableRow key={id}>
                      <TableCell>{item.specificDate}</TableCell>
                      <TableCell>
                        {item.times.map((t) => (
                          <Badge key={t} className="mr-1">
                            {t}
                          </Badge>
                        ))}
                      </TableCell>
                      <TableCell>{item.duration}</TableCell>
                      <TableCell>
                        <Badge
                          variant={
                            item.status === "Concluído"
                              ? "default"
                              : item.status === "Pendente"
                              ? "secondary"
                              : "outline"
                          }
                        >
                          {item.status}
                        </Badge>
                      </TableCell>
                      <TableCell className="text-right space-x-2">
                        <Button size="icon" variant="outline">
                          <Pencil size={16} />
                        </Button>
                        <Button size="icon" variant="destructive">
                          <Trash2 size={16} />
                        </Button>
                      </TableCell>
                    </TableRow>
                  ))}
                </TableBody>
              </Table>
            </div>
          </>
        )}
      </CardContent>
    </Card>
  );
}
